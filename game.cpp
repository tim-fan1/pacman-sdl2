#include <stdio.h>
#include <cassert>
#include <cmath>
#include "game.h"
#include <vector>
#include <utility>
#include <cstdlib>
#include <ctime>

Game::Game()
{
  window_ = NULL;
  renderer_ = NULL;
  spritesheet_ = NULL;
  board_ = NULL;
  pacman_ = NULL;
  blinky_ = NULL;
  inky_ = NULL;
  pinky_ = NULL;
  clyde_ = NULL;
  pellets_ = 0;
  totalPellets_ = 0;
  isGameOver_ = false;
  isGameOverWin_ = false;
  portalOneX = -1;
  portalOneY = -1;
  portalTwoX = -1;
  portalTwoY = -1;
  modes_ = NULL;
  currentModeIndex_ = 0;
  currentMode_ = false;

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
    window_ = SDL_CreateWindow("Pacman SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, TILE_SIZE * 28, TILE_SIZE * 36, 0);
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
    tempSurface = IMG_Load("/spritesheet.png");
    if (tempSurface == NULL) {
      printf("Failed to load image!\n");
      success = false;
    }
  }
  
  // And convert to texture.
  if (success) {
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
  if (spritesheet_ != NULL) SDL_DestroyTexture(spritesheet_);
  if (board_ != NULL) free(board_);
  if (pacman_ != NULL) delete pacman_;
  if (blinky_ != NULL) delete blinky_;
}

bool Game::getSDLInitSuccess()
{
  return success_;
}

bool Game::run(Level *level)
{
  /* Initialising game state. */

  // rand() is only used for frightened ghosts.
  srand((unsigned int)time(NULL));
  
  // Used by ghosts to find out how long to wait in current mode before switching.
  modes_ = (int *)malloc(8 * sizeof(int));
  if (modes_ == NULL) {
    printf("Failed to allocate memory for list of mode switching times!\n");
    return false;
  }
  modes_[0] = 7; modes_[1] = 20; // Wave 1.
  modes_[2] = 7; modes_[3] = 20; // Wave 2.
  modes_[4] = 5; modes_[5] = 20; // Wave 3.
  modes_[6] = 5; modes_[7] = -1; // Endless Wave.
  
  // Initialise timer used to track when to mode switch.
  timer_ = new Timer();
  
  // Walk through level text to build board and
  // place actors at their starting positions.
  boardHeight_ = level->getHeight();
  boardWidth_ = level->getWidth();
  std::string levelText = level->getLevelText();
  board_ = (TileType **)malloc(boardWidth_ * sizeof(TileType *));
  if (board_ == NULL) {
    printf("Failed to allocate memory for board of tiles!\n");
    return false;
  }
  for (int x = 0; x < boardWidth_; x++) {
    board_[x] = (TileType *)malloc(boardHeight_ * sizeof(TileType));
    if (board_[x] == NULL) {
      printf("Failed to allocate memory for row of tiles!\n");
      return false;
    }
  }
  for (int y = 0; y < boardHeight_; y++) {
    for (int x = 0; x < boardWidth_; x++) {
      char c = levelText.at(y * boardWidth_ + x);
      switch (c) {
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
        case '+': // Base.
          board_[x][y] = TILE_BASE;
          break;
        case 'g': // Gate of base.
          board_[x][y] = TILE_GATE;
          break;
        case 't': // Teleport, Tunnel.
          board_[x][y] = TILE_PORTAL;
          if (portalOneX == -1) {
            portalOneX = x;
            portalOneY = y;
          } else {
            portalTwoX = x;
            portalTwoY = y;
          }
          break;
        case '0': // PACMAN. At start stands below the base.
          board_[x][y] = TILE_NONE;
          if (pacman_ == NULL) {
            pacman_ = new Actor(x, y, TILE_SIZE);
          }
          break;
        case 'b': // Blinky. At start stands on a tile outside base,
                  // The tile that all ghosts target to reach home.
          board_[x][y] = TILE_NONE;
          if (blinky_ == NULL) {
            blinky_ = new Actor(x, y, TILE_SIZE, 0, x, y + 3);
          }
          break;
        case 'i': // Inky. At start stands inside base.
          board_[x][y] = TILE_BASE;
          if (inky_ == NULL) {
            inky_ = new Actor(x, y, TILE_SIZE, 500, x, y);
          }
          break;
        case 'p': // Pinky. At start stands inside base.
          board_[x][y] = TILE_BASE;
          if (pinky_ == NULL) {
            pinky_ = new Actor(x, y, TILE_SIZE, 0, x, y);
          }
          break;
        case 'c': // Clyde. At start stands inside base.
          board_[x][y] = TILE_BASE;
          if (clyde_ == NULL) {
            clyde_ = new Actor(x, y, TILE_SIZE, 1000, x, y);
          }
          break;
        default:
          printf("Unexpected character! '%c'\n", c);
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
      // Failed to update PACMAN to new direction.
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
    } else {
      printf("delay: %d\n", delay);
    }
    SDL_Delay(delay);
  }
  
  /* Exit game loop. */
  
  // Control reaches here when user has quit the application.
  return true;
}

void Game::gameOver(bool isWin)
{
  isGameOver_ = true;
  isGameOverWin_ = isWin;
}

bool Game::isCollidingWithActor(Actor *actorA, Actor *actorB)
{
  int aTileX = (actorA->getX() + (TILE_SIZE / 2)) / TILE_SIZE;
  int aTileY = (actorA->getY() + (TILE_SIZE / 2)) / TILE_SIZE;
  int bTileX = (actorB->getX() + (TILE_SIZE / 2)) / TILE_SIZE;
  int bTileY = (actorB->getY() + (TILE_SIZE / 2)) / TILE_SIZE;
  
  return (aTileX == bTileX && aTileY == bTileY);
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
      if (pacmanX + i < 0 || pacmanX + i > boardWidth_ - 1 ||
          pacmanY + j < 0 || pacmanY + j > boardHeight_ - 1) {
          continue;
      }
      tile = board_[pacmanX + i][pacmanY + j];
      if (isCollidingWithTile(pacman_, pacmanX + i, pacmanY + j)) {
        // Resolve collision.
        if (tile == TILE_WALL) {
          success = false;
          break;
        } else if (tile == TILE_GATE) {
          success = false;
          break;
        }
      }
    }
  }
  
  // If none of the tiles are walls, then successfully moved.
  if (success) {
    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        // Check if any of the tiles are pellets, power pellets, and so on.
        int tileX = pacmanX + i;
        int tileY = pacmanY + j;
        if (pacmanX + i < 0 || pacmanX + i > boardWidth_ - 1 ||
            pacmanY + j < 0 || pacmanY + j > boardHeight_ - 1) {
          continue;
        }
        tile = board_[tileX][tileY];
        if (isCollidingWithTile(pacman_, tileX, tileY)) {
          if (tile == TILE_PELLET) {
            pellets_ += 1;
            board_[pacmanX + i][pacmanY + j] = TILE_NONE;
            if (pellets_ == totalPellets_) {
              // PACMAN has collected all pellets.
              gameOver(true);
            }
          } else if (tile == TILE_POWER_PELLET) {
            pacman_->setPower(1000000);
            board_[pacmanX + i][pacmanY + j] = TILE_NONE;
            Actor *ghosts[4] = { blinky_, inky_, pinky_, clyde_ };
            for (int i = 0; i < 4; i++) {
              Actor *ghost = ghosts[i];
              if (ghost->getIsChase() || ghost->getIsScatter()) {
                ghost->setIsFrightened();
                ghost->turnAround();
              }
            }
          } else if (tile == TILE_PORTAL) {
            if (tileX * TILE_SIZE == pacman_->getX() &&
                tileY * TILE_SIZE == pacman_->getY()) {
              // We should be exactly in a portal.
              if (tileX == portalOneX && tileY == portalOneY) {
                pacman_->setTileX(portalTwoX);
                pacman_->setTileY(portalTwoY);
                pacman_->setDirection(DIRECTION_LEFT);
              } else if (tileX == portalTwoX && tileY == portalTwoY) {
                pacman_->setTileX(portalOneX);
                pacman_->setTileY(portalOneY);
                pacman_->setDirection(DIRECTION_RIGHT);
              } else {
                assert(!"We should be exactly in a portal...");
              }
            }
          }
        }
      }
    }
  }
  
  if (!success) {
    // If any of the tiles are walls, reverse PACMAN.
    pacman_->moveBackward();
  } else /* if (success) */ {
    // Successfully moved. Check if crash with any ghosts.
    Actor *ghosts[4] = { blinky_, inky_, pinky_, clyde_ };
    for (int i = 0; i < 4; i++) {
      Actor *ghost = ghosts[i];
      if (isCollidingWithActor(pacman_, ghost)) {
        // If Ghost is eaten, pass through it regardless of power.
        if (!ghost->getIsEaten()) {
          // If Ghost is not eaten, PACMAN survives only if Ghost
          // is Frightened, AND if PACMAN has remaining power left
          // to eat the Ghost.
          if (ghost->getIsFrightened() && (pacman_->getPower() > 0)) {
            ghost->setIsEaten();
          } else {
            gameOver(false);
          }
        }
      }
    }
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
      if (ghostX + i < 0 || ghostX + i > boardWidth_ - 1 ||
          ghostY + j < 0 || ghostY + j > boardHeight_ - 1) {
        continue;
      }
      tile = board_[ghostX + i][ghostY + j];
      if (isCollidingWithTile(ghost, ghostX + i, ghostY + j)) {
        // Resolve collision.
        if (tile == TILE_WALL) {
          success = false;
          break;
        } else if (tile == TILE_GATE) {
          // Can pass through gate only:
          if (ghost->getIsFindingSpot() || ghost->getIsFindingExit()) {
            // If are trying to enter or leave base.
            success = true;
          } else {
            // Otherwise, is just a normal wall.
            success = false;
          }
        }
      }
    }
  }
  
  if (!success) {
    // If any of the tiles are walls, reverse this ghost.
    ghost->moveBackward();
  } else {
    // Successfully moved. Check if this ghost crashed into PACMAN.
    if (isCollidingWithActor(ghost, pacman_)) {
      // If Ghost is eaten, pass through it regardless of power.
      if (!ghost->getIsEaten()) {
        // If Ghost is not eaten, PACMAN survives only if Ghost
        // is Frightened, AND if PACMAN has remaining power left
        // to eat the Ghost.
        if (ghost->getIsFrightened() && (pacman_->getPower() > 0)) {
          ghost->setIsEaten();
        } else {
          gameOver(false);
        }
      }
    }
    // And if this ghost has been eaten, Check if they have returned home.
    if (ghost->getIsEaten()) {
      if (ghost->getX() == (blinky_->getStartTileX() * TILE_SIZE) + (TILE_SIZE / 2) &&
          ghost->getY() == blinky_->getStartTileY() * TILE_SIZE) {
        // Ghost has arrived at the entrace of the homebase, Ghost is
        // no longer eaten. Instead, is now entering the homebase to
        // find its appropriate spot in the base. Once have arrived at
        // that spot, will then immediately start to escape, find exit.
        ghost->setIsFindingSpot();
      }
    }
    
    // If this ghost is in a portal, check if they are exactly in the portal.
    int ghostTileX = (ghost->getX() + (TILE_SIZE / 2)) / TILE_SIZE;
    int ghostTileY = (ghost->getY() + (TILE_SIZE / 2)) / TILE_SIZE;
    if (board_[ghostTileX][ghostTileY] == TILE_PORTAL) {
      if (ghostTileX * TILE_SIZE == ghost->getX() &&
          ghostTileY * TILE_SIZE == ghost->getY()) {
        // We should be exactly in a portal.
        if (ghostTileX == portalOneX && ghostTileY == portalOneY) {
          ghost->setTileX(portalTwoX);
          ghost->setTileY(portalTwoY);
        } else if (ghostTileX == portalTwoX && ghostTileY == portalTwoY) {
          ghost->setTileX(portalOneX);
          ghost->setTileY(portalOneY);
        } else {
          assert(!"We should be exactly in a portal...");
        }
      }
    }
  }
  
  return success;
}

void Game::setChaseOrScatter(Actor *ghost)
{
  if (currentMode_ == false) {
    ghost->setIsScatter();
  } else {
    ghost->setIsChase();
  }
}

void Game::moveGhost(Actor *ghost, int targetTileX, int targetTileY)
{
  int ghostTileX = (ghost->getX() + (TILE_SIZE / 2)) / TILE_SIZE;
  int ghostTileY = (ghost->getY() + (TILE_SIZE / 2)) / TILE_SIZE;

  // At the start of game, waiting until can start to leave home.
  if (ghost->getWaitingFrames() > 0) {
    ghost->setWaitingFrames(ghost->getWaitingFrames() - 1);
    // TODO: Move up and down within base, while waiting to leave home.
    return;
  } else if (ghost->getWaitingFrames() == 0) {
    // Start to leave home!
    ghost->setIsFindingExit();
    ghost->setWaitingFrames(-1);
  }
  
  if (ghost->getIsFindingExit()) {
    // Ghost is looking for exit.
    if (ghost->getX() == (blinky_->getStartTileX() * TILE_SIZE) + (TILE_SIZE / 2)
        &&
       (ghost->getY() == blinky_->getStartTileY() * TILE_SIZE)) {
      // Finished walking out of the base.
      setChaseOrScatter(ghost);
    } else if (ghost->getX() != (blinky_->getStartTileX() * TILE_SIZE) + (TILE_SIZE / 2)) {
      // If are not aligned with the gates, then continue
      // walking left/right until are aligned with the gates.
      if (ghost->getX() < (blinky_->getStartTileX() * TILE_SIZE) + (TILE_SIZE / 2)) {
        ghost->setDirection(DIRECTION_RIGHT);
      } else {
        ghost->setDirection(DIRECTION_LEFT);
      }
      moveGhostForwardWithCollision(ghost);
    } else /* if (ghost->getX() == (blinky_->getStartTileX() * TILE_SIZE) + (TILE_SIZE / 2)) */ {
      // If are aligned with the gates, then continue walking
      // up until have left the gates left the base.
      ghost->setDirection(DIRECTION_UP);
      moveGhostForwardWithCollision(ghost);
    }
  } else if (ghost->getIsFindingSpot()) {
    // Ghost is looking for their spot within the home base.
    if (ghost->getX() == ghost->getSpotInBaseX() &&
        ghost->getY() == ghost->getSpotInBaseY()) {
      // Have reached their spot in base. From next
      // frame onwards start looking for exit again.
      ghost->setIsFindingExit();
    } else if (ghost->getY() != pinky_->getSpotInBaseY()) {
      // Ghost is aiming for pinky's height.
      ghost->setDirection(DIRECTION_DOWN);
      moveGhostForwardWithCollision(ghost);
    } else {
      // Once at pinky's height, just move left or right until reach spot.
      if (ghost->getSpotInBaseX() < pinky_->getSpotInBaseX()) {
        ghost->setDirection(DIRECTION_LEFT);
      } else {
        ghost->setDirection(DIRECTION_RIGHT);
      }
      moveGhostForwardWithCollision(ghost);
    }
  } else if (ghost->getIsFrightened()) {
    if (ghost->getX() == ghostTileX * TILE_SIZE &&
        ghost->getY() == ghostTileY * TILE_SIZE) {
      // We are exactly on a tile. Make sure we are at an intersection.
      Direction oldDirection = ghost->getDirection();
      Direction newDirection = DIRECTION_NONE;
      int dx[4] = { 0, 0, -1, 1 };
      int dy[4] = { -1, 1, 0, 0 };
      std::vector<Direction> directions;
      for (int i = 0; i < 4; i++) {
        int adjacentX = ghostTileX + dx[i];
        int adjacentY = ghostTileY + dy[i];
        if (adjacentX < 0 || adjacentX > boardWidth_ - 1) continue;
        if (adjacentY < 0 || adjacentY > boardHeight_ - 1) continue;
        TileType adjacentTile = board_[adjacentX][adjacentY];
        if (i == 0 && oldDirection != DIRECTION_DOWN && adjacentTile != TILE_GATE && adjacentTile != TILE_WALL) {
          newDirection = DIRECTION_UP;
          directions.push_back(newDirection);
        } else if (i == 1 && oldDirection != DIRECTION_UP && adjacentTile != TILE_GATE && adjacentTile != TILE_WALL) {
          newDirection = DIRECTION_DOWN;
          directions.push_back(newDirection);
        } else if (i == 2 && oldDirection != DIRECTION_RIGHT && adjacentTile != TILE_GATE && adjacentTile != TILE_WALL) {
          newDirection = DIRECTION_LEFT;
          directions.push_back(newDirection);
        } else if (i == 3 && oldDirection != DIRECTION_LEFT && adjacentTile != TILE_GATE && adjacentTile != TILE_WALL) {
          newDirection = DIRECTION_RIGHT;
          directions.push_back(newDirection);
        }
      }
      // FIXME: The way I've designed directions and intersections is bad.
      assert(directions.size() != 0);
      while (1) {
        Direction randomDirection = directions.at(rand() % directions.size());
        ghost->setDirection(randomDirection);
        if (moveGhostForwardWithCollision(ghost)) break;
      }
    } else {
      moveGhostForwardWithCollision(ghost);
    }
  } else {
    bool justFlipped = false;
    
    // Checking if this ghost should transition between
    // the Chase/Scatters state on this frame.
    // If the ghost does change state on this frame,
    // then we will also flip them around.
    if (ghost->getIsScatter()) {
      setChaseOrScatter(ghost);
      if (ghost->getIsChase()) {
        ghost->turnAround();
        justFlipped = true;
      }
    } else if (ghost->getIsChase()) {
      setChaseOrScatter(ghost);
      if (ghost->getIsScatter()) {
        ghost->turnAround();
        justFlipped = true;
      }
    }
    
    // Follow the given target tile, provided by caller.
    // If Ghost has just flipped, then we shouldn't calculate a new direction;
    // The direction we just flipped to should be the direction we are headed,
    // rather than any direction related to reaching the given target tile.
    if (justFlipped == false ||
       (ghost->getX() == ghostTileX * TILE_SIZE &&
          ghost->getY() == ghostTileY * TILE_SIZE) ||
       (ghost->getX() == (blinky_->getStartTileX() * TILE_SIZE) + (TILE_SIZE / 2) &&
          ghost->getY() == blinky_->getStartTileY() * TILE_SIZE)) {
      // If have arrived completely into a new tile, so are exactly in
      // the tile that they are in right now, Or, If are right outside
      // the front gates of the home base, which is where Blinky starts,
      Direction oldDirection = ghost->getDirection();
      Direction newDirection = DIRECTION_NONE;
      std::vector<std::pair<float, Direction>> directions;
      int dx[4] = { 0, 0, -1, 1 };
      int dy[4] = { -1, 1, 0, 0 };
      for (int i = 0; i < 4; i++) {
        float distanceFromAdjacentTile =
          sqrt(pow(ghostTileX + dx[i] - targetTileX, 2) +
               pow(ghostTileY + dy[i] - targetTileY, 2));
        if (i == 0 && oldDirection != DIRECTION_DOWN) {
          newDirection = DIRECTION_UP;
          directions.push_back(
            std::make_pair(distanceFromAdjacentTile, newDirection));
        } else if (i == 1 && oldDirection != DIRECTION_UP) {
          newDirection = DIRECTION_DOWN;
          directions.push_back(
            std::make_pair(distanceFromAdjacentTile, newDirection));
        } else if (i == 2 && oldDirection != DIRECTION_RIGHT) {
          newDirection = DIRECTION_LEFT;
          directions.push_back(
            std::make_pair(distanceFromAdjacentTile, newDirection));
        } else if (i == 3 && oldDirection != DIRECTION_LEFT) {
          newDirection = DIRECTION_RIGHT;
          directions.push_back(
            std::make_pair(distanceFromAdjacentTile, newDirection));
        }
      }
      std::sort(directions.begin(), directions.end());
      for (auto vit = directions.begin(); vit != directions.end(); vit++) {
        ghost->setDirection(vit->second);
        if (moveGhostForwardWithCollision(ghost)) {
          break;
        }
      }
    } else {
      // Otherwise, continue moving this ghost forward towards the tile
      // in front of it. Will eventually be exactly in that tile.
      moveGhostForwardWithCollision(ghost);
    }
  }
}

bool Game::update(Direction newDirection)
{
  /* No point updating if game is over. */
  if (isGameOver_) {
    return true;
  }

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
  
  // Game starts when PACMAN starts moving. Don't
  // move ghosts if PACMAN hasn't started moving.
  if (pacman_->getDirection() == DIRECTION_NONE) {
    currentMode_ = false; // Ghosts start off in Scatter mode.
    currentModeIndex_ = 0;
  } else /* if (pacman_->getDirection() != DIRECTION_NONE) */ {
    if (!timer_->isRunning()) {
      // We start the timer for the first time when we know
      // for sure that Player has started controlling PACMAN.
      timer_->start();
    } else /* if (timer_->isRunning()) */ {
      // Check if ghosts should be mode switching on this frame using timer.
      if (modes_[currentModeIndex_] == -1) {
        // Ghosts should be staying in chase mode on this frame.
        currentMode_ = true;
        printf("endless wave!\n");
      } else if (timer_->getTimePassedSecs() > modes_[currentModeIndex_]) {
        // Ghosts should switch to opposite mode on this frame.
        currentModeIndex_++;
        currentMode_ = !currentMode_;
        printf("mode switch! ");
        if (currentMode_) {
          printf("chase!\n");
        } else {
          printf("scatter!\n");
        }
        // Re-Start timer to count how long the new mode should last.
        timer_->stop();
        timer_->start();
      }
    }
    int pacmanTileX = (pacman_->getX() + (TILE_SIZE / 2)) / TILE_SIZE;
    int pacmanTileY = (pacman_->getY() + (TILE_SIZE / 2)) / TILE_SIZE;
    int targetTileX = pacmanTileX;
    int targetTileY = pacmanTileY;
    
    // Moving Blinky. Target is directly where PACMAN is.
    if (blinky_->getIsEaten()) {
      // Head back to the entrance of the home base,
      // Which is where Blinky starts at start of game.
      targetTileX = blinky_->getStartTileX();
      targetTileY = blinky_->getStartTileY();
    } else if (blinky_->getIsScatter()) {
      // Target the top right corner.
      targetTileX = boardWidth_ - 3;
      targetTileY = 0;
    } else /* if (blinky_->getIsChase()) */ {
      // Head for PACMAN!
      targetTileX = pacmanTileX;
      targetTileY = pacmanTileY;
    }
    moveGhost(blinky_, targetTileX, targetTileY);
    
    // Moving Inky. Flanks PACMAN with Blinky.
    if (inky_->getIsEaten()) {
      // Head back to the entrance of the home base,
      // Which is where Blinky starts at start of game.
      targetTileX = blinky_->getStartTileX();
      targetTileY = blinky_->getStartTileY();
    } else if (inky_->getIsScatter()) {
      // Target the bottom right corner.
      targetTileX = boardWidth_ - 1;
      targetTileY = boardHeight_ - 2;
    } else /* if (inky_->getIsChase()) */ {
      // Head for behind PACMAN so that we can flank PACMAN with Blinky!
      // Get the tile two tiles in front of PACMAN, this will be the midpoint.
      int midTileX = pacmanTileX;
      int midTileY = pacmanTileY;
      switch (pacman_->getDirection()) {
        case DIRECTION_LEFT:
          // Shift mX left two tiles.
          midTileX -= 2;
          break;
        case DIRECTION_UP:
          // Shift mY up two tiles.
          midTileY -= 2;
          break;
        case DIRECTION_DOWN:
          // Shift mY down two tiles.
          midTileY += 2;
          break;
        case DIRECTION_RIGHT:
          // Shift mX right two tiles.
          midTileX += 2;
          break;
        default:
          break;
      }
      // Get where blinky is.
      int blinkyTileX = (blinky_->getX() + (TILE_SIZE / 2)) / TILE_SIZE;
      int blinkyTileY = (blinky_->getY() + (TILE_SIZE / 2)) / TILE_SIZE;
      
      // Inky's target tile follows this math.
      // Inky's target tile, the midpoint tile, and blinky's current tile,
      // form a straight line segment, where the midpoint tile is equidistant
      // from the other two tiles.
      targetTileX = (2 * midTileX) - blinkyTileX;
      targetTileY = (2 * midTileY) - blinkyTileY;
    }
    moveGhost(inky_, targetTileX, targetTileY);
    
    // Moving Pinky. Target is in front of PACMAN,
    // So that we can attack PACMAN from in front.
    if (pinky_->getIsEaten()) {
      // Head back to the entrance of the home base,
      // Which is where Blinky starts at start of game.
      targetTileX = blinky_->getStartTileX();
      targetTileY = blinky_->getStartTileY();
    } else if (pinky_->getIsScatter()) {
      // Target the top left corner.
      targetTileX = 2;
      targetTileY = 0;
    } else /* if (pinky_->getIsChase()) */ {
      // Head for four tiles in front of PACMAN,
      // so we can attack PACMAN from in front.
      targetTileX = pacmanTileX;
      targetTileY = pacmanTileY;
      switch (pacman_->getDirection()) {
        case DIRECTION_LEFT:
          // Shift tX left four tiles.
          targetTileX -= 4;
          break;
        case DIRECTION_UP:
          // Shift tY up four tiles.
          targetTileY -= 4;
          break;
        case DIRECTION_DOWN:
          // Shift tY down four tiles.
          targetTileY += 4;
          break;
        case DIRECTION_RIGHT:
          // Shift tX right four tiles.
          targetTileX += 4;
          break;
        default:
          break;
      }
    }
    moveGhost(pinky_, targetTileX, targetTileY);
    
    // Moving Clyde. Target is PACMAN, until get close
    // then start running away into Clyde's corner.
    if (clyde_->getIsEaten()) {
      // Head back to the entrance of the home base,
      // Which is where Blinky starts at start of game.
      targetTileX = blinky_->getStartTileX();
      targetTileY = blinky_->getStartTileY();
    } else if (clyde_->getIsScatter()) {
      // Target the bottom left corner.
      targetTileX = 0;
      targetTileY = boardHeight_ - 2;
    } else /* if (clyde_->getIsChase()) */ {
      // Head for PACMAN, until we are within 8 tiles of
      // distance from PACMAN, where we then start running
      // away from PACMAN into our corner. PACMAN will be
      // very confused!!!
      int clydeTileX = (clyde_->getX() + (TILE_SIZE / 2)) / TILE_SIZE;
      int clydeTileY = (clyde_->getY() + (TILE_SIZE / 2)) / TILE_SIZE;
      int distanceFromPacman = (int)sqrt(pow(clydeTileX - pacmanTileX,2) +
                                         pow(clydeTileY - pacmanTileY,2));
      if (distanceFromPacman >= (8 * TILE_SIZE)) {
        // When far away from PACMAN, chase PACMAN.
        targetTileX = pacmanTileX;
        targetTileY = pacmanTileY;
      } else {
        // And when get close to PACMAN, run
        // away from PACMAN into corner.
        targetTileX = 0;
        targetTileY = boardHeight_ - 2;
      }
    }
    moveGhost(clyde_, targetTileX, targetTileY);
  }
  
  // Frame is finished. Drain PACMAN of one frame of power.
  if (pacman_->getPower() > 0) {
    // If PACMAN has power left, reduce by one frame.
    pacman_->setPower(pacman_->getPower() - 1);
  }
  if (pacman_->getPower() == 0) {
    pacman_->setPower(-1);
    // Pacman has run out of power,
    // Un-Frighten all ghosts before next frame starts.
    Actor *ghosts[4] = { blinky_, inky_, pinky_, clyde_ };
    for (int i = 0; i < 4; i++) {
      Actor *ghost = ghosts[i];
      if (ghost->getIsFrightened()) {
        setChaseOrScatter(ghost);
      }
    }
  }
  
  return success;
}

void Game::render()
{
  // Clear buffer.
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0xFF);
  SDL_RenderClear(renderer_);
  
  // Draw board into buffer.
  for (int j = 0; j < boardHeight_; j++) {
    for (int i = 0; i < boardWidth_; i++) {
      switch (board_[i][j]) {
        case TILE_NONE:
          break;
        case TILE_WALL:
          drawWall(i * TILE_SIZE, j * TILE_SIZE);
          break;
        case TILE_POWER_PELLET:
          drawPowerPellet(i * TILE_SIZE, j * TILE_SIZE);
          break;
        case TILE_PELLET:
          drawPellet(i * TILE_SIZE, j * TILE_SIZE);
          break;
        case TILE_GATE:
        case TILE_BASE:
//          drawWall(i * TILE_SIZE, j * TILE_SIZE);
          break;
        case TILE_PORTAL:
          break;
        default:
          printf("Render: Unexpected tile!\n");
          break;
      }
    }
  }
  
  // Draw actors into buffer, on top of board.
  drawPacman();
  drawGhost(inky_);
  drawGhost(pinky_);
  drawGhost(clyde_);
  drawGhost(blinky_);
  
  // And draw game over text on top, if is game over.
  // TODO: Make the game over screen nicer.
  if (isGameOver_) {
    if (isGameOverWin_) {
      SDL_SetRenderDrawColor(renderer_, 0x00, 0x00, 0x00, 0xFF);
      SDL_RenderClear(renderer_);
    } else {
      SDL_SetRenderDrawColor(renderer_, 0x33, 0x33, 0x33, 0xFF);
      SDL_RenderClear(renderer_);
    }
  }
    
  // Present buffer.
  SDL_RenderPresent(renderer_);
  return;
}

/* Taking from the 48px spritesheet. */

void Game::drawPacman() {
  drawPowerPellet(pacman_->getX(), pacman_->getY());

}

void Game::drawGhost(Actor *ghost) {
  SDL_Rect srcrect = { .x = 0, .y = 48, .w = 48, .h = 48 };
  if (!ghost->getIsEaten()) {
    if (ghost == blinky_) {
      srcrect.x += (0 * 48);
    } else if (ghost == inky_) {
      srcrect.x += (1 * 48);
    } else if (ghost == pinky_) {
      srcrect.x += (2 * 48);
    } else {
      srcrect.x += (3 * 48);
    }
    drawSprite(&srcrect, ghost->getX() - (TILE_SIZE / 2), ghost->getY() - (TILE_SIZE / 2));
  }
  
  srcrect = { .x = 0, .y = 0, .w = 48, .h = 48 };
  if (ghost->getDirection() == DIRECTION_UP) {
    srcrect.x += (1 * 48);
  } else if (ghost->getDirection() == DIRECTION_LEFT) {
    srcrect.x += (3 * 48);
  } else if (ghost->getDirection() == DIRECTION_RIGHT) {
    srcrect.x += (4 * 48);
  } else {
    srcrect.x += (2 * 48);
  }
  drawSprite(&srcrect, ghost->getX() - (TILE_SIZE / 2), ghost->getY() - (TILE_SIZE / 2));
}

/* Taking from the 24px spritesheet. */

void Game::drawWall(int x, int y)
{
  SDL_Rect srcrect = { .x = (8 * 48), .y = 0, .w = 24, .h = 24 };
  // TODO: do a better properly working drawing wall algorithm.
  int tileX = x / TILE_SIZE; int tileY = y / TILE_SIZE;
  
  // Get the indices of the adjacent tiles.
  int upTileX = tileX; int upTileY = tileY - 1;
  int downTileX = tileX; int downTileY = tileY + 1;
  int leftTileX = tileX - 1; int leftTileY = tileY;
  int rightTileX = tileX + 1; int rightTileY = tileY;
  
  // Make sure none are out of bounds.
  assert(board_[0][0] == TILE_NONE);
  if (upTileY < 0) {
    upTileX = 0;
    upTileY = 0;
  }
  if (downTileY > boardHeight_ - 1) {
    downTileX = 0;
    downTileY = 0;
  }
  if (leftTileX < 0) {
    leftTileX = 0;
    leftTileY = 0;
  }
  if (rightTileX > boardWidth_ - 1) {
    rightTileX = 0;
    rightTileY = 0;
  }

  // Get the type of each adjacent tile.
  TileType upTile = board_[upTileX][upTileY];
  TileType downTile = board_[downTileX][downTileY];
  TileType leftTile = board_[leftTileX][leftTileY];
  TileType rightTile = board_[rightTileX][rightTileY];
  
  if (leftTile == TILE_WALL && rightTile == TILE_WALL) {
    // Drawing walls.
    srcrect.x += 48;
    if (upTile != TILE_WALL) {
      // Draw bottom wall.
      srcrect.x += 24;
      srcrect.y += 24;
    } else if (downTile != TILE_WALL) {
      // Draw top wall.
      srcrect.x += 0;
      srcrect.y += 0;
    }
  } else if (upTile == TILE_WALL && downTile == TILE_WALL) {
    // Drawing walls.
    srcrect.x += 48;
    if (leftTile != TILE_WALL) {
      // Draw right wall.
      srcrect.x += 24;
      srcrect.y += 0;
    } else if (rightTile != TILE_WALL) {
      // Draw left wall.
      srcrect.x += 0;
      srcrect.y += 24;
    }
  } else if (downTile == TILE_WALL) {
    // Drawing corners.
    srcrect.x += 0;
    if (rightTile == TILE_WALL) {
      // Drawing top-left corner.
      srcrect.x += 0;
      srcrect.y += 0;
    } else if (leftTile == TILE_WALL) {
      // Drawing top-right corner.
      srcrect.x += 24;
      srcrect.y += 0;
    }
  } else if (upTile == TILE_WALL) {
    // Drawing corners.
    srcrect.x += 0;
    if (rightTile == TILE_WALL) {
      // Drawing bottom-left corner.
      srcrect.x += 0;
      srcrect.y += 24;
    } else if (leftTile == TILE_WALL) {
      // Drawing bottom-right corner.
      srcrect.x += 24;
      srcrect.y += 24;
    }
  }
  drawSprite(&srcrect, x, y);
}

void Game::drawPellet(int x, int y)
{
  SDL_Rect srcrect = { .x = (5 * 48), .y = 0, .w = 24, .h = 24 };
  drawSprite(&srcrect, x, y);
}

void Game::drawPowerPellet(int x, int y)
{
  SDL_Rect srcrect = { .x = (5 * 48) + (24), .y = 0, .w = 24, .h = 24 };
  drawSprite(&srcrect, x, y);
}

/* Helper function to draw from different spritesheets. */

bool Game::drawSprite(SDL_Rect *clip, int x, int y)
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
    .w = clip->w, .h = clip->h
  };
  
  // Copy the sprite into our renderer's buffer.
  if (SDL_RenderCopy(renderer_, spritesheet_, &srcrect, &dstrect) != 0) {
    success = false;
    printf("Failed to render sprite at (%d,%d)! SDL Error: %s\n",
           x, y, SDL_GetError());
  }
  
  return success;
}

