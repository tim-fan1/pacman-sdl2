#include <stdio.h>
#include <cassert>
#include <cmath>
#include "game.h"
#include <vector>
#include <utility>
#include <cstdlib>
#include <ctime>
#include "level.h"
#include "actor.h"

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
  frightenedGhostSprite_ = { .x = 0, .y = 0, .w = 48, .h = 48 };
  numFramesPassed = 0;
  averageFrameTime = 0;

  success_ = false;
  
  bool success = true;
  
  /* Initialising SDL. */
  
  if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
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
  
  // Free temp resource.
  SDL_FreeSurface(tempSurface);
  
  /* Initialising game state. */
  
  // rand() is only used for frightened ghosts.
  if (success) srand((unsigned int)time(NULL));
  
  // Used by ghosts to find out how long to wait in current mode before switching.
  if (success) {
    modes_ = (int *)malloc(8 * sizeof(int));
    if (modes_ == NULL) {
      printf("Failed to allocate memory for list of mode switching times!\n");
      success = false;
    }
  }
  if (success) {
    modes_[0] = 7; modes_[1] = 20; // Wave 1. 7 seconds scatter, 20 seconds chase.
    modes_[2] = 7; modes_[3] = 20; // Wave 2. 7 seconds scatter, 20 seconds chase.
    modes_[4] = 5; modes_[5] = 20; // Wave 3. 5 seconds scatter, 20 seconds chase.
    modes_[6] = 5; modes_[7] = -1; // Endless Wave. 7 seconds scatter, endless chase.
  }
  
  // Initialise timer used to track when to mode switch.
  if (success) timer_ = new Timer();
  
  // Get default level and allocate space for game board.
  Level *level = new Level();
  if (success) {
    boardHeight_ = level->getHeight();
    boardWidth_ = level->getWidth();
    board_ = (TileType **)malloc(boardWidth_ * sizeof(TileType *));
    if (board_ == NULL) {
      printf("Failed to allocate memory for board of tiles!\n");
      success = false;
    }
  }
  if (success) {
    for (int x = 0; x < boardWidth_; x++) {
      board_[x] = (TileType *)malloc(boardHeight_ * sizeof(TileType));
      if (board_[x] == NULL) {
        printf("Failed to allocate memory for row of tiles!\n");
        success = false;
      }
    }
  }
  
  // Build game board.
  if (success) for (int y = 0; y < boardHeight_; y++) {
    std::string levelText = level->getLevelText();
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
            pacman_ = new Actor(x, y, TILE_SIZE, DIRECTION_NONE);
          }
          break;
        case 'b': // Blinky. At start stands on a tile outside base,
                  // The tile that all ghosts target to reach home.
          board_[x][y] = TILE_NONE;
          if (blinky_ == NULL) {
            blinky_ = new Actor(x, y, TILE_SIZE, DIRECTION_LEFT, 0, x, y + 3);
          }
          break;
        case 'i': // Inky. At start stands inside base.
          board_[x][y] = TILE_BASE;
          if (inky_ == NULL) {
            inky_ = new Actor(x, y, TILE_SIZE, DIRECTION_UP, 30, x, y);
          }
          break;
        case 'p': // Pinky. At start stands inside base.
          board_[x][y] = TILE_BASE;
          if (pinky_ == NULL) {
            pinky_ = new Actor(x, y, TILE_SIZE, DIRECTION_DOWN, 10, x, y);
          }
          break;
        case 'c': // Clyde. At start stands inside base.
          board_[x][y] = TILE_BASE;
          if (clyde_ == NULL) {
            clyde_ = new Actor(x, y, TILE_SIZE, DIRECTION_UP, 90, x, y);
          }
          break;
        default:
          printf("Unexpected character! '%c'\n", c);
          break;
      }
    }
  }
  
  // Free temp resource.
  delete level;
  
  // Set to let caller know that initialisation succeeded.
  success_ = success;
  return;

}

Game::~Game()
{
  // For drawing.
  if (window_ != NULL) SDL_DestroyWindow(window_);
  if (renderer_ != NULL) SDL_DestroyRenderer(renderer_);
  if (spritesheet_ != NULL) SDL_DestroyTexture(spritesheet_);
  SDL_Quit();
  
  // For running simulation.
  if (board_ != NULL) free(board_);
  if (modes_ != NULL) free(modes_);
  if (timer_ != NULL) delete timer_;
  if (pacman_ != NULL) delete pacman_;
  if (blinky_ != NULL) delete blinky_;
  if (inky_ != NULL) delete inky_;
  if (pinky_ != NULL) delete pinky_;
  if (clyde_ != NULL) delete clyde_;
}

bool Game::getSuccess()
{
  return success_;
}

bool Game::run()
{
  if (getSuccess() == false) {
    return false;
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
      pacmanAnimationFrame_ = 0;
    }
    
    if (pacman_->getDirection() != DIRECTION_NONE) {
      // Update PACMAN's animation frame every five frames.
      pacmanAnimationFrameCounter_++;
      if (pacmanAnimationFrameCounter_ % 5 == 0) {
        pacmanAnimationFrameCounter_ = 0;
        // Update PACMAN's animation frame.
        pacmanAnimationFrame_ = (pacmanAnimationFrame_ == 0) ? 1 : 0;
      }
    }
    
    pelletAnimationFrameCounter_++;
    if (pelletAnimationFrameCounter_ % 10 == 0) {
      pelletAnimationFrameCounter_ = 0;
      // Update pellet's animation frame.
      pelletAnimationFrame_ = (pelletAnimationFrame_ == 0) ? 1 : 0;
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
    averageFrameTime = ((numFramesPassed * averageFrameTime) + realFrameTime) / (numFramesPassed + 1);
    numFramesPassed++;
    printf("average frame time: %lf\n", averageFrameTime);
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
  
  // TODO: Set up game state so that we can reset and restart the level.
}

bool Game::isCollidingWithActor(Actor *actorA, Actor *actorB)
{
//  if (actorA == pacman_ || actorB == pacman_) return false;
  int aTileX = actorA->getTileX();
  int aTileY = actorA->getTileY();
  int bTileX = actorB->getTileX();
  int bTileY = actorB->getTileY();
  
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
  // TODO: Use, WHERE IS PACMAN WITHIN THE TILE IT IS IN?, to detect if collision.
  //
  // First try move PACMAN forward without worrying about collision.
  pacman_->moveForward();
  
  // Which tile is the center of PACMAN now on?
  int pacmanX = pacman_->getTileX();
  int pacmanY = pacman_->getTileY();
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
  if (success) for (int i = -1; i <= 1; i++) for (int j = -1; j <= 1; j++) {
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
        Actor *ghosts[4] = { blinky_, inky_, pinky_, clyde_ };
        for (int i = 0; i < 4; i++) {
          Actor *ghost = ghosts[i];
          if (ghost->getWaitingPellets() > 0) {
            ghost->setWaitingPellets(ghost->getWaitingPellets() - 1);
          }
        }
        board_[pacmanX + i][pacmanY + j] = TILE_NONE;
        if (pellets_ == totalPellets_) {
          // PACMAN has collected all pellets.
          gameOver(true);
        }
      } else if (tile == TILE_POWER_PELLET) {
        // Power pellet last 6 seconds, 6000 milliseconds.
        pacman_->setPower(6000 / FRAME_TIME);
        board_[pacmanX + i][pacmanY + j] = TILE_NONE;
        Actor *ghosts[4] = { blinky_, inky_, pinky_, clyde_ };
        for (int i = 0; i < 4; i++) {
          Actor *ghost = ghosts[i];
          GHOST_STATE state = ghost->getState();
          if (state == GHOST_CHASE || state == GHOST_SCATTER) {
            ghost->setState(GHOST_FRIGHTENED);
            frightenedGhostSprite_ = { .x = 0, .y = 0, .w = 48, .h = 48 };
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
        if (ghost->getState() != GHOST_EATEN) {
          // If Ghost is not eaten, PACMAN survives only if Ghost
          // is Frightened, AND if PACMAN has remaining power left
          // to eat the Ghost.
          if (ghost->getState() == GHOST_FRIGHTENED && (pacman_->getPower() > 0)) {
            ghost->setState(GHOST_EATEN);
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
  int ghostX = ghost->getTileX();
  int ghostY = ghost->getTileY();
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
          GHOST_STATE state = ghost->getState();
          if (state == GHOST_FINDING_SPOT || state == GHOST_FINDING_EXIT) {
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
      if (ghost->getState() != GHOST_EATEN) {
        // If Ghost is not eaten, PACMAN survives only if Ghost
        // is Frightened, AND if PACMAN has remaining power left
        // to eat the Ghost.
        if (ghost->getState() == GHOST_FRIGHTENED && (pacman_->getPower() > 0)) {
          ghost->setState(GHOST_EATEN);
        } else {
          gameOver(false);
        }
      }
    }
    // And if this ghost has been eaten, Check if they have returned home.
    if (ghost->getState() == GHOST_EATEN) {
      if (ghost->getX() == (blinky_->getStartTileX() * TILE_SIZE) + (TILE_SIZE / 2) &&
          ghost->getY() == blinky_->getStartTileY() * TILE_SIZE) {
        // Ghost has arrived at the entrace of the homebase, Ghost is
        // no longer eaten. Instead, is now entering the homebase to
        // find its appropriate spot in the base. Once have arrived at
        // that spot, will then immediately start to escape, find exit.
        ghost->setState(GHOST_FINDING_SPOT);
      }
    }
    
    // If this ghost is in a portal, check if they are exactly in the portal.
    int ghostTileX = ghost->getTileX();
    int ghostTileY = ghost->getTileY();
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
    ghost->setState(GHOST_SCATTER);
  } else {
    ghost->setState(GHOST_CHASE);
  }
}

void Game::moveGhost(Actor *ghost, int targetTileX, int targetTileY)
{
  ghost->setTargetTileX(targetTileX);
  ghost->setTargetTileY(targetTileY);
  int ghostTileX = ghost->getTileX();
  int ghostTileY = ghost->getTileY();

  // At the start of game, waiting until can start to leave home.
  if (ghost->getWaitingPellets() == 0) {
    // Start to leave home!
    ghost->setState(GHOST_FINDING_EXIT);
    ghost->setWaitingPellets(-1);
  } else if (ghost->getWaitingPellets() > 0){
    // Continue going forward and back, bouncing
    // against walls while waiting to leave base.
    if (!moveGhostForwardWithCollision(ghost)) {
      ghost->turnAround();
      moveGhostForwardWithCollision(ghost);
    }
    return;
  }
  
  assert(ghost->getWaitingPellets() == -1);
  
  GHOST_STATE state = ghost->getState();
  if (state == GHOST_FINDING_EXIT) {
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
  } else if (state == GHOST_FINDING_SPOT) {
    // Ghost is looking for their spot within the home base.
    if (ghost->getX() == ghost->getSpotInBaseX() &&
        ghost->getY() == ghost->getSpotInBaseY()) {
      // Have reached their spot in base. From next
      // frame onwards start looking for exit again.
      ghost->setState(GHOST_FINDING_EXIT);
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
  } else if (state == GHOST_FRIGHTENED) {
    if (ghost->getX() == ghostTileX * TILE_SIZE &&
        ghost->getY() == ghostTileY * TILE_SIZE) {
      // FIXME: can be not exactly in a tile and will have to choose next direction to turn.
      // We are exactly on a tile. Make sure we are at an intersection.
      // If are at an intersection, then randomly choose a direction to turn,
      // either left, right, or continue forward.
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
      while (directions.size() != 0) {
        int randomIndex = rand() % directions.size();
        Direction randomDirection = directions.at(randomIndex);
        ghost->setDirection(randomDirection);
        if (moveGhostForwardWithCollision(ghost)) {
          // Successfully moved forward, so current direction is a valid direction.
          break;
        } else {
          // Current direction leads to a wall, is not a valid direction.
          // Remove from list so that on next iteration of loop
          // this direction is not considered as a valid direction.
          directions.erase(directions.begin() + randomIndex);
        }
      }
    } else {
      // Not at an intersection. Keep moving forward until are at an intersection.
      moveGhostForwardWithCollision(ghost);
    }
  } else {
    // Case for if scatter or chase or eaten.
    bool justFlipped = false;
    
    // Checking if this ghost should transition between
    // the Chase/Scatters state on this frame.
    // If the ghost does change state on this frame,
    // then we will also flip them around.
    if (ghost->getState() == GHOST_SCATTER) {
      setChaseOrScatter(ghost);
      if (ghost->getState() == GHOST_CHASE) {
        ghost->turnAround();
        justFlipped = true;
      }
    } else if (ghost->getState() == GHOST_CHASE) {
      setChaseOrScatter(ghost);
      if (ghost->getState() == GHOST_SCATTER) {
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
    int pacmanTileX = pacman_->getTileX();
    int pacmanTileY = pacman_->getTileY();
    int targetTileX = pacmanTileX;
    int targetTileY = pacmanTileY;
    
    // Moving Blinky. Target is directly where PACMAN is.
    switch (blinky_->getState()) {
    case GHOST_EATEN:
      // Head back to the entrance of the home base,
      // Which is where Blinky starts at start of game.
      targetTileX = blinky_->getStartTileX();
      targetTileY = blinky_->getStartTileY();
      break;
    case GHOST_SCATTER:
      // Target the top right corner.
      targetTileX = boardWidth_ - 3;
      targetTileY = 0;
      break;
    case GHOST_CHASE:
      // Head for PACMAN!
      targetTileX = pacmanTileX;
      targetTileY = pacmanTileY;
      break;
    case GHOST_FINDING_EXIT:
      targetTileX = blinky_->getStartTileX();
      targetTileX = blinky_->getStartTileY();
      break;
    case GHOST_FINDING_SPOT:
      targetTileX = blinky_->getSpotInBaseX();
      targetTileY = blinky_->getSpotInBaseY();
      break;
    default:
      targetTileX = -1;
      targetTileY = -1;
    }
    moveGhost(blinky_, targetTileX, targetTileY);
    
    // Moving Inky. Flanks PACMAN with Blinky.
    switch (inky_->getState()) {
    case GHOST_EATEN:
      // Head back to the entrance of the home base,
      // Which is where Blinky starts at start of game.
      targetTileX = blinky_->getStartTileX();
      targetTileY = blinky_->getStartTileY();
    case GHOST_SCATTER:
      // Target the bottom right corner.
      targetTileX = boardWidth_ - 1;
      targetTileY = boardHeight_ - 2;
    case GHOST_FINDING_EXIT:
      targetTileX = blinky_->getStartTileX();
      targetTileX = blinky_->getStartTileY();
      break;
    case GHOST_FINDING_SPOT:
      targetTileX = inky_->getSpotInBaseX();
      targetTileY = inky_->getSpotInBaseY();
      break;
    default:
      targetTileX = -1;
      targetTileY = -1;
    }
    if (inky_->getState() == GHOST_CHASE) {
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
    switch (pinky_->getState()) {
    case GHOST_EATEN:
      // Head back to the entrance of the home base,
      // Which is where Blinky starts at start of game.
      targetTileX = blinky_->getStartTileX();
      targetTileY = blinky_->getStartTileY();
      break;
    case GHOST_SCATTER:
      // Target the top left corner.
      targetTileX = 2;
      targetTileY = 0;
      break;
    case GHOST_FINDING_EXIT:
      targetTileX = blinky_->getStartTileX();
      targetTileX = blinky_->getStartTileY();
    case GHOST_FINDING_SPOT:
      targetTileX = pinky_->getSpotInBaseX();
      targetTileY = pinky_->getSpotInBaseY();
    default:
      targetTileX = -1;
      targetTileY = -1;
    }
    if (pinky_->getState() == GHOST_CHASE) {
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
    switch (pinky_->getState()) {
    case GHOST_EATEN:
      // Head back to the entrance of the home base,
      // Which is where Blinky starts at start of game.
      targetTileX = blinky_->getStartTileX();
      targetTileY = blinky_->getStartTileY();
      break;
    case GHOST_SCATTER:
      // Target the bottom left corner.
      targetTileX = 0;
      targetTileY = boardHeight_ - 2;
      break;
    case GHOST_FINDING_EXIT:
      targetTileX = blinky_->getStartTileX();
      targetTileX = blinky_->getStartTileY();
    case GHOST_FINDING_SPOT:
      targetTileX = clyde_->getSpotInBaseX();
      targetTileY = clyde_->getSpotInBaseY();
    default:
      targetTileX = -1;
      targetTileY = -1;
    }
    if (clyde_->getState() == GHOST_CHASE) {
      // Head for PACMAN, until we are within 8 tiles of
      // distance from PACMAN, where we then start running
      // away from PACMAN into our corner. PACMAN will be
      // very confused!!!
      int clydeTileX = clyde_->getTileX();
      int clydeTileY = clyde_->getTileY();
      int distanceFromPacman = (int)sqrt(pow(clydeTileX - pacmanTileX, 2) +
                                         pow(clydeTileY - pacmanTileY, 2));
      if (distanceFromPacman >= 8) {
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
    // Flash frightened ghosts 3 times in the last 1.5 seconds.
    if (pacman_->getPower() == (1500 / FRAME_TIME)) {
      frightenedGhostSprite_ = { .x = 48, .y = 2 * 48, .w = 48, .h = 48 };
    } else if (pacman_->getPower() == (1250 / FRAME_TIME)) {
      frightenedGhostSprite_ = { .x = 0, .y = 0, .w = 48, .h = 48 };
    } else if (pacman_->getPower() == (1000 / FRAME_TIME)) {
      frightenedGhostSprite_ = { .x = 48, .y = 2 * 48, .w = 48, .h = 48 };
    } else if (pacman_->getPower() == (750 / FRAME_TIME)) {
      frightenedGhostSprite_ = { .x = 0, .y = 0, .w = 48, .h = 48 };
    } else if (pacman_->getPower() == (500 / FRAME_TIME)) {
      frightenedGhostSprite_ = { .x = 48, .y = 2 * 48, .w = 48, .h = 48 };
    } else if (pacman_->getPower() == (250 / FRAME_TIME)) {
      frightenedGhostSprite_ = { .x = 0, .y = 0, .w = 48, .h = 48 };
    }
  }
  if (pacman_->getPower() == 0) {
    pacman_->setPower(-1);
    // Pacman has run out of power,
    // Un-Frighten all ghosts before next frame starts.
    Actor *ghosts[4] = { blinky_, inky_, pinky_, clyde_ };
    for (int i = 0; i < 4; i++) {
      Actor *ghost = ghosts[i];
      if (ghost->getState() == GHOST_FRIGHTENED) {
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
          drawGate(i * TILE_SIZE, j * TILE_SIZE);
          break;
        case TILE_BASE:
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
  drawGhost(blinky_);
  drawGhost(inky_);
  drawGhost(pinky_);
  drawGhost(clyde_);
  
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
  SDL_Rect srcrect = { .x = (4 * 48), .y = 48, .w = 48, .h = 48 };
  if (pacmanAnimationFrame_ == 0) {
    srcrect.x += (0 * 48);
  } else if (pacmanAnimationFrame_ == 1) {
    srcrect.x += (1 * 48);
  }
  double angle = 0;
  if (pacman_->getDirection() == DIRECTION_UP) {
    angle = 270;
  } else if (pacman_->getDirection() == DIRECTION_DOWN) {
    angle = 90;
  } else if (pacman_->getDirection() == DIRECTION_LEFT) {
    angle = 180;
  } else if (pacman_->getDirection() == DIRECTION_RIGHT) {
    angle = 0;
  } else {
    srcrect.x = (5 * 48);
  }
  drawSprite(&srcrect, pacman_->getX() - (TILE_SIZE / 2), pacman_->getY() - (TILE_SIZE / 2), angle);
}

void Game::drawGhost(Actor *ghost) {
  // Drawing ghost sprite.
  SDL_Rect srcrect = { .x = 0, .y = 48, .w = 48, .h = 48 };
  if (ghost->getState() == GHOST_FRIGHTENED) {
    // Draw frightened ghost sprite.
    srcrect = frightenedGhostSprite_;
    drawSprite(&srcrect, ghost->getX() - (TILE_SIZE / 2), ghost->getY() - (TILE_SIZE / 2));
  } else if (ghost->getState() != GHOST_EATEN) {
    // Draw normal ghost sprite.
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
  } else {
    // Draw nothing.
  }
  
  // Drawing target tile on screen for debugging.
  // FIXME: Remove me! Is buggy (target tile is wrong when ghost first leaving base).
  srcrect = { .x = 0, .y = (2 * 48), .w = 24, .h = 24 };
  if (ghost == blinky_) {
    srcrect.x += (0 * 24);
  } else if (ghost == inky_) {
    srcrect.x += (1 * 24);
  } else if (ghost == pinky_) {
    srcrect.y += (1 * 24);
  } else {
    srcrect.x += (1 * 24);
    srcrect.y += (1 * 24);
  }
  drawSprite(&srcrect, ghost->getTargetTileX() * TILE_SIZE, ghost->getTargetTileY() * TILE_SIZE);
  
  // Drawing ghost eyes.
  if (ghost->getState() != GHOST_FRIGHTENED) {
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
}

/* Taking from the 24px spritesheet. */

void Game::drawGate(int x, int y)
{
  SDL_Rect srcrect = { .x = (5 * 48), .y = 24, .w = 24, .h = 24 };
  drawSprite(&srcrect, x, y);
}

void Game::drawWall(int x, int y)
{
  SDL_Rect srcrect = { .x = (6 * 48), .y = 0, .w = 24, .h = 24 };
  int tileX = x / TILE_SIZE; int tileY = y / TILE_SIZE;
  
  // Get the indices of all surrounding tiles.
  
  // The Top and Left tiles.
  int topTileX = tileX; int topTileY = tileY - 1;
  int leftTileX = tileX - 1; int leftTileY = tileY;
  
  // The Top-Left tile.
  int topLeftTileX = leftTileX; int topLeftTileY = topTileY;
  
  // The Bot and Right tiles.
  int botTileX = tileX; int botTileY = tileY + 1;
  int rightTileX = tileX + 1; int rightTileY = tileY;
  
  // The Bot-Right tile.
  int botRightTileX = rightTileX; int botRightTileY = botTileY;
  
  // Remaining surrounding tiles.
  int botLeftTileX = leftTileX; int botLeftTileY = botTileY;
  int topRightTileX = rightTileX; int topRightTileY = topTileY;
  
  // Make sure none are out of bounds.
  assert(board_[0][3] == TILE_WALL);
  
  int *arrayX[8] = { &topTileX, &leftTileX, &botTileX, &rightTileX, &topLeftTileX, &botLeftTileX, &topRightTileX, &botRightTileX };
  int *arrayY[8] = { &topTileY, &leftTileY, &botTileY, &rightTileY, &topLeftTileY, &botLeftTileY, &topRightTileY, &botRightTileY };

  for (int i = 0; i < 8; i++) {
    int aX = *arrayX[i];
    int aY = *arrayY[i];
    if ((aX < 0) || (aX > boardWidth_ - 1)) {
      (*arrayX[i]) = 0;
      (*arrayY[i]) = 3;
    }
    if ((aY < 0) || (aY > boardHeight_ - 1)) {
      (*arrayX[i]) = 0;
      (*arrayY[i]) = 3;
    }
  }

  // Get the type of each surrounding tile.
  TileType topTile = board_[topTileX][topTileY];
  TileType botTile = board_[botTileX][botTileY];
  TileType leftTile = board_[leftTileX][leftTileY];
  TileType rightTile = board_[rightTileX][rightTileY];
  
  TileType topLeftTile = board_[topLeftTileX][topLeftTileY];
  TileType botLeftTile = board_[botLeftTileX][botLeftTileY];
  TileType topRightTile = board_[topRightTileX][topRightTileY];
  TileType botRightTile = board_[botRightTileX][botRightTileY];
  
  bool success = false;
  
  /* See if this wall should be drawn as one of the corner tiles. */
  
  if (isWall(botTile)) {
    // Checking for Top-Left corner wall and Top-Right corner wall.
    if (isWall(rightTile)) {
      // Is a Top-Left corner wall candidate. Check if it should be.
      if (!isWall(botRightTile)) {
        // Is a Top-Left corner wall!
        success = true;
        srcrect = { .x = (6 * 48), .y = 0, .w = 24, .h = 24 };
      } else if (!isWall(topLeftTile) && !isWall(topTile) && !isWall(leftTile)) {
        // Is a Top-Left corner wall!
        success = true;
        srcrect = { .x = (6 * 48), .y = 0, .w = 24, .h = 24 };
      }
    }
    if (!success && isWall(leftTile)) {
      // Is a Top-Right corner wall candidate. Check if it should be.
      if (!isWall(botLeftTile)) {
        // Is a Top-Right corner wall!
        success = true;
        srcrect = { .x = (6 * 48) + 24, .y = 0, .w = 24, .h = 24 };
      } else if (!isWall(topRightTile) && !isWall(topTile) && !isWall(rightTile)) {
        // Is a Top-Right corner wall!
        success = true;
        srcrect = { .x = (6 * 48) + 24, .y = 0, .w = 24, .h = 24 };
      }
    }
  }
  
  if (!success && isWall(topTile)) {
    // Checking for Bot-Left corner wall and Bot-Right corner wall.
    if (isWall(rightTile)) {
      // Is a Bot-Left corner wall candidate. Check if it should be.
      if (!isWall(topRightTile)) {
        // Is a Bot-Left corner wall!
        success = true;
        srcrect = { .x = (6 * 48), .y = 24, .w = 24, .h = 24 };
      } else if (!isWall(botLeftTile) && !isWall(botTile) && !isWall(leftTile)) {
        // Is a Bot-Left corner wall!
        success = true;
        srcrect = { .x = (6 * 48), .y = 24, .w = 24, .h = 24 };
      }
    }
    if (!success && isWall(leftTile)) {
      // Is a Bot-Right corner wall candidate. Check if it should be.
      if (!isWall(topLeftTile)) {
        // Is a Bot-Right corner wall!
        success = true;
        srcrect = { .x = (6 * 48) + 24, .y = 24, .w = 24, .h = 24 };
      } else if (!isWall(botRightTile) && !isWall(botTile) && !isWall(rightTile)) {
        // Is a Bot-Right corner wall!
        success = true;
        srcrect = { .x = (6 * 48) + 24, .y = 24, .w = 24, .h = 24 };
      }
    }
  }
  
  /* See if this wall should be drawn as one of the edge tiles. */
  
  if (!success) {
    if (!isWall(leftTile) || !isWall(rightTile)) {
      // Draw Left/Right edge wall tile.
      srcrect = { .x = (7 * 48), .y = 24, .w = 24, .h = 24 };
    } else {
      // Draw Top/Bot edge wall tile.
      srcrect = { .x = (7 * 48), .y = 0, .w = 24, .h = 24 };
    }
  }
  
  drawSprite(&srcrect, x, y);
}

bool Game::isWall(TileType tile)
{
  return (tile == TILE_WALL || tile == TILE_GATE);
}

void Game::drawPellet(int x, int y)
{
  SDL_Rect srcrect = { .x = (5 * 48), .y = 0, .w = 24, .h = 24 };
  drawSprite(&srcrect, x, y);
}

void Game::drawPowerPellet(int x, int y)
{
  SDL_Rect srcrect = { .x = (5 * 48) + (24), .y = 0, .w = 24, .h = 24 };
  if (pelletAnimationFrame_ == 0) {
    srcrect.y += (0 * 24);
  } else if (pelletAnimationFrame_ == 1) {
    srcrect.y += (1 * 24);
  }
  drawSprite(&srcrect, x, y);
}

/* Helper function to draw from different spritesheets. */

bool Game::drawSprite(SDL_Rect *clip, int x, int y, double angle)
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
  if (SDL_RenderCopyEx(renderer_, spritesheet_, &srcrect, &dstrect, angle, NULL, SDL_FLIP_NONE) != 0) {
    success = false;
    printf("Failed to render sprite at (%d,%d)! SDL Error: %s\n",
           x, y, SDL_GetError());
  }
  
  return success;
}

