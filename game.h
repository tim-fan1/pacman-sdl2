#ifndef game_h
#define game_h

#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>

#include "level.h"
#include "actor.h"
#include "direction.h"

typedef enum {
  TILE_NONE, TILE_WALL, TILE_PELLET, TILE_POWER_PELLET
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
     */
    void run(Level *level);
    
  private:
  
    bool isCollidingWithTile(Actor *actor, int tileX, int tileY);

    /**
     * Move PACMAN forward while accounting for collision
     * within our game world.
     *
     * \Returns If PACMAN collided with any walls.
     */
    bool movePacmanForwardWithCollision();

    bool moveGhostForwardWithCollision(Actor *ghost);

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
    
    /**
     * Clip the spritesheet using given clip, and
     * draw that sprite at (x,y) on our window --
     * add to our renderer's buffer -- stretched
     * to be width w and height h.
     */
    bool drawSprite(SDL_Rect *clip, int x, int y, int w, int h);

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
    int pellets_;
    int totalPellets_;
    
    // How long each frame should take in ms.
    static const Uint32 FRAME_TIME = 20;
    static const Uint32 TILE_SIZE = 25;
    
    // TODO: remove these.
    void drawRed(int x, int y);
    void drawYellow(int x, int y);
    void drawBlue(int x, int y);
    void drawGreen(int x, int y);
};

#endif /* game_h */
