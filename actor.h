#ifndef actor_h
#define actor_h

#include "direction.h"

class Actor {
  public:
    // Which tile should this actor start on in the game board?
    Actor(int tileX, int tileY, int tileSize, int speed = 5);
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
    bool getIsFrightened();
    void setIsFrightened(bool isFrightened);
    
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

    bool isFrightened_;
    
    /* For PACMAN. */
    
    // How many frames of power does this PACMAN have left?
    int power_;
};

#endif /* actor_h */
