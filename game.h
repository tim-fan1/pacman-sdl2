#ifndef game_h
#define game_h

#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>

#include "level.h"
#include "actor.h"
#include "direction.h"
#include "timer.h"

typedef enum {
  TILE_NONE,
  TILE_WALL,
  TILE_PELLET,
  TILE_POWER_PELLET,
  TILE_PORTAL,
  TILE_BASE,
  TILE_GATE
} TileType;

class Game {
  public:
    /**
     * Initialises SDL modules. Get SDL initialisation success through
     * Game::getSDLInitSuccess().
     */
    Game();
    ~Game();
    bool getSDLInitSuccess();
    
    /**
     * Start running game, initially rendering the given level.
     *
     * \Returns If game started running successfully.
     */
    bool run(Level *level);
    
  private:
    void gameOver(bool isWin);
  
    bool isCollidingWithActor(Actor *actorA, Actor *actorB);
  
    bool isCollidingWithTile(Actor *actor, int tileX, int tileY);

    /**
     * Move PACMAN forward while accounting for collision
     * within our game world.
     *
     * \Returns If PACMAN successfully moved forward.
     */
    bool movePacmanForwardWithCollision();

    bool moveGhostForwardWithCollision(Actor *ghost);
    
    void moveGhost(Actor *ghost, int targetTileX, int targetTileY);
    
    /**
     * Use Timer to calculate if this ghost should be in Chase or Scatter mode.
     */
    void setChaseOrScatter(Actor *ghost);

    /**
     * Update our simulation by one frame:
     *
     * - The player wants PACMAN to change to the given direction on this
     *   frame, and then move forward one step in the new direction.
     *
     * - If PACMAN is unable to move forward one step in the new direction,
     *   then don't change to new direction, remain in old direction, and
     *   move forward one step (if are able to) in the old direction.
     *
     * - If PACMAN does successfully change direction, the user wants
     *   PACMAN to remain in this direction, until their next successful
     *   request for a change in direction.
     *
     * \Returns If PACMAN successfully changed direction to the new direction.
     */
    bool update(Direction newDirection);

    /**
     * Render the current state of our simulation to screen.
     */
    void render();
    
    void drawGhost(Actor *ghost);
    
    void drawPacman();
    
    /**
     * Clip the appropriate spritesheet using given clip,
     * and draw that sprite at (x,y) on our window.
     */
    bool drawSprite(SDL_Rect *clip, int x, int y);

    void drawWall(int x, int y);
    void drawPowerPellet(int x, int y);
    void drawPellet(int x, int y);

    // SDL init success.
    bool success_;
    
    // For drawing our simulation to screen.
    SDL_Window *window_;
    SDL_Renderer *renderer_;
    SDL_Texture *spritesheet_;
    
    // For running our simulation.
    TileType **board_;
    int boardWidth_;
    int boardHeight_;
    Actor *pacman_;
    Actor *blinky_;
    Actor *inky_;
    Actor *pinky_;
    Actor *clyde_;
    int pellets_;
    int totalPellets_;
    bool isGameOver_;
    bool isGameOverWin_;
    int portalOneX;
    int portalOneY;
    int portalTwoX;
    int portalTwoY;
    Timer *timer_;
    int *modes_;           // How long to stay in each mode:
    int currentModeIndex_; // Even indices is Scatter mode,
                           // Odd indices is Chase mode.
    bool currentMode_;     // The current mode all out-of-base alive
                           // ghosts should be in on this frame.
                           // false is Scatter mode,
                           // true is Chase mode.
    
    // How long each frame should take in ms.
    static const Uint32 FRAME_TIME = 16.7;
    static const Uint32 TILE_SIZE = 24;


};

#endif /* game_h */
