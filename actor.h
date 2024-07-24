#ifndef actor_h
#define actor_h

#include "direction.h"

typedef enum {
  GHOST_NONE,
  GHOST_CHASE,
  GHOST_SCATTER,
  GHOST_EATEN,
  GHOST_FRIGHTENED,
  GHOST_FINDING_SPOT,
  GHOST_FINDING_EXIT
} GHOST_STATE;

class Actor {
  public:
    /**
     * Places actor in the middle of the two adjacent tiles
     * (tileX, tileY) and ((tileX + 1), tileY), so this actor
     * will have pixel coordinates (coords of the top left corner):
     *   x = (tileX x TILESIZE) + (TILESIZE / 2),
     *   y = (tileY x TILESIZE).
     * We do this so that when we draw this actor on the screen -- but
     * with having double the height and double the width of its actual
     * hitbox!!! -- the sprite drawn on the screen will end up filling
     * up both tiles (tileX, tileY) and (tileX + 1, tileY) completely.
     */
    Actor(int tileX, int tileY, int tileSize, int waitingFrames = 0,
          int inBaseTileX = 0, int inBaseTileY = 0, int speed = 3);
    ~Actor();
    
    // Where is this actor (the top left corner) in pixel space?
    int getX();
    int getY();
    
    // Where is this actor's starting position in tile space?
    int getStartTileX();
    int getStartTileY();
    
    void setTileX(int tileX);
    void setTileY(int tileY);
    
    // Which direction is this actor facing?
    Direction getDirection();
    void setDirection(Direction direction);

    // Move one frame in current direction.
    void moveForward();
    void moveBackward();
    
    /* For ghosts. */
    
    void turnAround();
    
    int getSpotInBaseX();
    int getSpotInBaseY();
    
    void setTargetTileX(int targetTileX);
    void setTargetTileY(int targetTileY);
    int getTargetTileX();
    int getTargetTileY();
    
    // The four main states.
    void printState();
    
    bool getIsChase();
    void setIsChase();
    
    bool getIsScatter();
    void setIsScatter();
    
    bool getIsEaten();
    void setIsEaten();
    
    bool getIsFrightened();
    void setIsFrightened();
    
    // Entering home.
    bool getIsFindingSpot();
    void setIsFindingSpot();
    
    // Leaving home.
    bool getIsFindingExit();
    void setIsFindingExit();
    
    // At start of game, how long to wait until start to leave home.
    int getWaitingFrames();
    void setWaitingFrames(int waitingFrames);
    
    void setSpeed(int speed);
    
    /* For PACMAN. */
    int getPower();
    void setPower(int powerFrames);

  private:
    // Where is this actor (the top left corner) in pixel space?
    int x_;
    int y_;
    
    // Where did this actor start in tile space?
    int startTileX_;
    int startTileY_;

    // Which direction is this actor facing?
    Direction direction_;
    
    // Speed in pixels/frame.
    int speed_;
    
    int tileSize_;
    
    /* For ghosts. */
    GHOST_STATE state_;
    
    // How many more frames does this ghost have to wait inside home base?
    int waitingFrames_;
    
    // Where should this ghost go inside the home base when return to base.
    int inBaseTileX_;
    int inBaseTileY_;
    
    int targetTileX_;
    int targetTileY_;
    
    /* For PACMAN. */
    
    // How many frames of power does this PACMAN have left?
    int power_;
};

#endif /* actor_h */
