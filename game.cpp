#include <stdio.h>
#include <cassert>
#include <cmath>
#include "game.h"
#include <vector>
#include <utility>

Game::Game()
{
  window_ = NULL;
  renderer_ = NULL;
  spritesheet_ = NULL;
  board_ = NULL;
  pacman_ = NULL;
  blinky_ = NULL;
  pellets_ = 0;
  totalPellets_ = 0;

  bool success = true;
  
  // Init SDL.
  if (success && SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
    printf("SDL video initialisation failed! SDL Error %s\n", SDL_GetError());
    success = false;
  }
  if (success && IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
    printf("SDL_image initialisation failed! SDL Error %s\n", SDL_GetError());
    success = false;
  }
  if (success && TTF_Init() == -1) {
    printf("SDL_ttf initialisation failed! SDL Error %s\n", SDL_GetError());
    success = false;
  }
  
  // Initialise pacman-sdl2 application window and its screen renderer.
  if (success) {
    window_ = SDL_CreateWindow("Pacman SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 800, 0);
    if (window_ == NULL) {
      printf("pacman-sdl2 application window initialisation failed! SDL Error %s\n", SDL_GetError());
      success = false;
    }
  }
  if (success) {
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (renderer_ == NULL) {
      printf("pacman-sdl2 application window renderer could not be created! SDL Error %s\n", SDL_GetError());
      success = false;
    }
  }
  
  // Get spritesheet image.
  SDL_Surface *tempSurface = NULL;
  if (success) {
    tempSurface = IMG_Load("/dots.png");
    if (tempSurface == NULL) {
      printf("Failed to load image!\n");
      success = false;
    }
  }
  
  // And convert to texture.
  if (success) {
    SDL_SetColorKey(tempSurface, SDL_TRUE,
                    SDL_MapRGB(tempSurface->format, 0, 0xFF, 0xFF));
    spritesheet_ = SDL_CreateTextureFromSurface(renderer_, tempSurface);
    if (spritesheet_ == NULL) {
      printf("Failed to convert image to texture! SDL Error: %s\n",
             SDL_GetError());
      success = false;
    }
  }
  
  // Free resources.
  SDL_FreeSurface(tempSurface);
  
  // Set to let caller know whether initialisation succeeded.
  success_ = success;
}

Game::~Game()
{
  if (window_ != NULL) SDL_DestroyWindow(window_);
  SDL_Quit();
}

bool Game::getSDLInitSuccess()
{
  return success_;
}

void Game::run(Level *level)
{
  /* Initialising game state. */

  // Walk through level text to build board and
  // place actors at their starting positions.
  boardHeight_ = level->getHeight();
  boardWidth_ = level->getWidth();
  std::string levelText = level->getLevelText();
  board_ = (TileType **)malloc(boardWidth_ * sizeof(TileType *));
  for (int y = 0; y < boardHeight_; y++) {
    board_[y] = (TileType *)malloc(boardHeight_ * sizeof(TileType));
  }
  for (int y = 0; y < boardHeight_; y++) {
    for (int x = 0; x < boardWidth_; x++) {
      switch (levelText.at(y * boardHeight_ + x)) {
        case '-':
          board_[x][y] = TILE_NONE;
          break;
        case '#':
          board_[x][y] = TILE_WALL;
          break;
        case 'x':
          board_[x][y] = TILE_PELLET;
          totalPellets_ += 1;
          break;
        case 'y':
          board_[x][y] = TILE_POWER_PELLET;
          break;
        case '0':
          // PACMAN starting position.
          board_[x][y] = TILE_NONE;
          pacman_ = new Actor(x, y, TILE_SIZE);
          break;
        case '1':
          // Ghost starting position.
          board_[x][y] = TILE_NONE;
          blinky_ = new Actor(x, y, TILE_SIZE);
          break;
        default:
          printf("Error: Unexpected character!\n");
          board_[x][y] = TILE_NONE;
          break;
      }
    }
  }
  
  /* Run game loop. */

  // Keep the screen on, until user requests to quit.
  bool quit = false;
  SDL_Event event;
  
  Direction turnBuffer = DIRECTION_NONE;

  // For each frame.
  while (!quit) {
    // Marking when this frame starts, so we can find how
    // long it took for this frame to update and render.
    Uint32 start = SDL_GetTicks();
    
    // To store if user inputted a direction on this frame.
    // If user inputted more than one direction on this frame,
    // we will store only the most recent one.
    Direction direction = DIRECTION_NONE;
  
    // Poll for user input.
    while (SDL_PollEvent(&event) != 0) {
      if (event.type == SDL_QUIT) {
        // User requests to quit.
        quit = true;
      } else if (event.type == SDL_KEYDOWN) {
        // User requests to change direction.
        switch (event.key.keysym.sym) {
          case SDLK_UP:
            direction = DIRECTION_UP;
            break;
          case SDLK_DOWN:
            direction = DIRECTION_DOWN;
            break;
          case SDLK_LEFT:
            direction = DIRECTION_LEFT;
            break;
          case SDLK_RIGHT:
            direction = DIRECTION_RIGHT;
            break;
          default:
            break;
        }
      } else if (event.type == SDL_KEYUP) {
        switch (event.key.keysym.sym) {
          case SDLK_UP:
          case SDLK_DOWN:
          case SDLK_LEFT:
          case SDLK_RIGHT:
            // Clear turn buffer.
            turnBuffer = DIRECTION_NONE;
            break;
          default:
            break;
        }
      }
    }
    
    // If user didn't input a new direction this frame,
    // take from the turn buffer.
    if (direction == DIRECTION_NONE) {
      direction = turnBuffer;
    }
    
    // Update our simulation by one frame.
    if (!update(direction)) {
      // Failed to update to new direction.
      // Remember the direction, will use in future frames
      // if user doesn't input a direction on those frames.
      turnBuffer = direction;
    }
    
    // Render the current state of our simulation to screen.
    render();
    
    // Maintaining a consistent frame rate.
    Uint32 realFrameTime = SDL_GetTicks() - start;
    Uint32 delay = FRAME_TIME - realFrameTime;
    if (realFrameTime > FRAME_TIME) {
      // In case we took too long to update and render on this frame.
      printf("we are slowwwww\n");
      delay = 0;
    }
    SDL_Delay(delay);
  }
  
  /* Exit game loop. */
  
  // Control reaches here when user has quit the application.
  return;
}

bool Game::isCollidingWithTile(Actor *actor, int tileX, int tileY)
{
  // How far away the top side of the actor is from the top of the screen.
  int topActor = actor->getY();
  // How far away the bottom side of the actor is from the top of the screen.
  int botActor = topActor + TILE_SIZE;
  // How far away the left side of the actor is from the left of the screen.
  int leftActor = actor->getX();
  // How far away the right side of the actor is from the left of the screen.
  int rightActor = leftActor + TILE_SIZE;
  
  // How far away the top side of the tile is from the top of the screen.
  int topTile = (tileY * TILE_SIZE);
  // How far away the bottom side of the tile is from the top of the screen.
  int botTile = topTile + TILE_SIZE;
  // How far away the left side of the tile is from the left of the screen.
  int leftTile = (tileX * TILE_SIZE);
  // How far away the right side of the tile is from the left of the screen.
  int rightTile = leftTile + TILE_SIZE;
  
  if (botActor <= topTile) return false;
  if (botTile <= topActor) return false;
  
  if (rightActor <= leftTile) return false;
  if (rightTile <= leftActor) return false;
  
  return true;
}

bool Game::movePacmanForwardWithCollision()
{
  bool success = true;
  
  // First try move PACMAN forward without worrying about collision.
  pacman_->moveForward();
  
  // Which tile is the center of PACMAN now on?
  int pacmanX = (pacman_->getX() + (TILE_SIZE / 2)) / TILE_SIZE;
  int pacmanY = (pacman_->getY() + (TILE_SIZE / 2)) / TILE_SIZE;
  TileType tile;

  // Get all the tiles surrounding that tile.
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      // And check if any are walls.
      tile = board_[pacmanX + i][pacmanY + j];
      if (isCollidingWithTile(pacman_, pacmanX + i, pacmanY + j)) {
        // Resolve collision.
        if (tile == TILE_WALL) {
          success = false;
          break;
        }
      }
    }
  }
  
  // If don't crash with any walls.
  if (success) {
    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        // Check if any of the tiles are pellets, power pellets, and so on.
        tile = board_[pacmanX + i][pacmanY + j];
        if (isCollidingWithTile(pacman_, pacmanX + i, pacmanY + j)) {
          if (tile == TILE_PELLET) {
            pellets_ += 1;
            board_[pacmanX + i][pacmanY + j] = TILE_NONE;
            if (pellets_ == totalPellets_) {
              // TODO: Cleared the level!
              pacman_->setDirection(DIRECTION_NONE);
            }
          } else if (tile == TILE_POWER_PELLET) {
            // TODO: Implement power pellet.
          }
        }
      }
    }
  }
  
  if (!success) {
    // If any of the tiles are walls, reverse PACMAN.
    pacman_->moveBackward();
  } else {
    // TODO: Successfully moved. Check if crashed into a ghost!
//    assert(!"TODO: Check if crashed into ghost!");
  }
  
  return success;
}

bool Game::moveGhostForwardWithCollision(Actor *ghost)
{
  bool success = true;
  
  // Move this ghost forward without worrying about collision.
  ghost->moveForward();
  
  // Which tile is the center of this ghost now on?
  int ghostX = (ghost->getX() + (TILE_SIZE / 2)) / TILE_SIZE;
  int ghostY = (ghost->getY() + (TILE_SIZE / 2)) / TILE_SIZE;
  TileType tile;

  // Get all the tiles surrounding that tile.
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      // And check if any are walls.
      tile = board_[ghostX + i][ghostY + j];
      if (isCollidingWithTile(ghost, ghostX + i, ghostY + j)) {
        // Resolve collision.
        if (tile == TILE_WALL) {
          success = false;
          break;
        }
      }
    }
  }
  
  if (!success) {
    // If any of the tiles are walls, reverse this ghost.
    ghost->moveBackward();
  } else {
    // TODO: Successfully moved. Check if crashed into PACMAN!
//    assert(!"TODO: Check if crashed into PACMAN!");
  }
  
  return success;
}

bool Game::update(Direction newDirection)
{
  bool success = true;
  
  /* First move PACMAN. */
  
  // Moving PACMAN.
  Direction oldDirection = pacman_->getDirection();
  if (newDirection == DIRECTION_NONE) {
    success = movePacmanForwardWithCollision();
  } else /* if (newDirection != DIRECTION_NONE) */ {
    pacman_->setDirection(newDirection);
    if (!movePacmanForwardWithCollision()) {
      // There was a collision with a wall. Change back
      // to old direction and try move forward again.
      success = false;
      pacman_->setDirection(oldDirection);
      movePacmanForwardWithCollision();
    }
  }
  
  /* Then move the ghosts. */
  
  // Moving Blinky, target is where PACMAN is.
  int blinkyTileX = (blinky_->getX() + (TILE_SIZE / 2)) / TILE_SIZE;
  int blinkyTileY = (blinky_->getY() + (TILE_SIZE / 2)) / TILE_SIZE;
  int targetTileX = (pacman_->getX() + (TILE_SIZE / 2)) / TILE_SIZE;
  int targetTileY = (pacman_->getY() + (TILE_SIZE / 2)) / TILE_SIZE;

  if (blinky_->getX() == blinkyTileX * TILE_SIZE &&
      blinky_->getY() == blinkyTileY * TILE_SIZE) {
    // Blinky is exactly in the tile it is in.
    // Decide which tile Blinky will go to next.
    oldDirection = blinky_->getDirection();
    std::vector<std::pair<float, Direction>> directions;
    int dx[4] = { 0, 0, -1, 1 };
    int dy[4] = { -1, 1, 0, 0 };
    for (int i = 0; i < 4; i++) {
      float distanceFromAdjacentTile =
        sqrt(pow(blinkyTileX + dx[i] - targetTileX, 2) +
             pow(blinkyTileY + dy[i] - targetTileY, 2));
      if (i == 0 && oldDirection != DIRECTION_DOWN) {
        newDirection = DIRECTION_UP;
      } else if (i == 1 && oldDirection != DIRECTION_UP) {
        newDirection = DIRECTION_DOWN;
      } else if (i == 2 && oldDirection != DIRECTION_RIGHT) {
        newDirection = DIRECTION_LEFT;
      } else if (i == 3 && oldDirection != DIRECTION_LEFT) {
        newDirection = DIRECTION_RIGHT;
      }
      directions.push_back(std::make_pair(distanceFromAdjacentTile, newDirection));
    }
    std::sort(directions.begin(), directions.end());
    for (auto vit = directions.begin(); vit != directions.end(); vit++) {
      blinky_->setDirection(vit->second);
      if (moveGhostForwardWithCollision(blinky_)) {
        break;
      }
    }
  } else {
    moveGhostForwardWithCollision(blinky_);
  }
  
  return success;
}

void Game::render()
{
  // Clear buffer.
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0xFF);
  SDL_RenderClear(renderer_);
  
  // Draw board into buffer.
  for (int i = 0; i < boardHeight_; i++) {
    for (int j = 0; j < boardWidth_; j++) {
      switch (board_[i][j]) {
        case TILE_NONE:
          break;
        case TILE_WALL:
          drawRed(i * TILE_SIZE, j * TILE_SIZE);
          break;
        case TILE_POWER_PELLET:
          drawBlue(i * TILE_SIZE, j * TILE_SIZE);
          break;
        case TILE_PELLET:
          drawGreen(i * TILE_SIZE, j * TILE_SIZE);
          break;
        default:
          printf("Render: Unexpected tile!\n");
          break;
      }
    }
  }
  
  // Draw actors into buffer, on top of board.
  drawYellow(pacman_->getX(), pacman_->getY());
  drawYellow(blinky_->getX(), blinky_->getY());
  
  // Present buffer.
  SDL_RenderPresent(renderer_);
  return;
}

void Game::drawRed(int x, int y)
{
  SDL_Rect rect = { .x = x, .y = y, .w = TILE_SIZE, .h = TILE_SIZE };
  SDL_SetRenderDrawColor(renderer_, 0xDD, 0x00, 0x00, 0xFF);
  SDL_RenderFillRect(renderer_, &rect);
  SDL_RenderDrawRect(renderer_, &rect);
}
void Game::drawYellow(int x, int y)
{
  SDL_Rect rect = { .x = x, .y = y, .w = TILE_SIZE, .h = TILE_SIZE };
  SDL_SetRenderDrawColor(renderer_, 0x00, 0xFF, 0xFF, 0xFF);
  SDL_RenderFillRect(renderer_, &rect);
  SDL_RenderDrawRect(renderer_, &rect);
}
void Game::drawBlue(int x, int y)
{
  SDL_Rect srcrect = { .x = 100, .y = 100, .w = 100, .h = 100 };
  drawSprite(&srcrect, x, y, TILE_SIZE, TILE_SIZE);
}
void Game::drawGreen(int x, int y)
{
  SDL_Rect srcrect = { .x = 100, .y = 0, .w = 100, .h = 100 };
  drawSprite(&srcrect, x, y, TILE_SIZE, TILE_SIZE);
}

bool Game::drawSprite(SDL_Rect *clip, int x, int y, int w, int h)
{
  assert(clip != NULL);
  
  bool success = true;
  
  // Clip the spritesheet to get the sprite.
  // The sprite is at (clip.x, clip.y) on the spritesheet.
  // The width of the sprite is given by (clip.w, clip.h).
  SDL_Rect srcrect = {
    .x = clip->x, .y = clip->y,
    .w = clip->w, .h = clip->h
  };
  
  // And paste that sprite at the given (x,y) on the
  // screen, stretched to be width w and height h.
  SDL_Rect dstrect = {
    .x = x, .y = y,
    .w = w, .h = h
  };
  
  // Copy the sprite into our renderer's buffer.
  if (SDL_RenderCopy(renderer_, spritesheet_, &srcrect, &dstrect) != 0) {
    success = false;
    printf("Failed to render sprite at (%d,%d)! SDL Error: %s\n",
           x, y, SDL_GetError());
  }
  
  return success;
}
