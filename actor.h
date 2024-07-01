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
    
    // Which direction is this actor facing?
    Direction getDirection();
    void setDirection(Direction direction);

    // Move one frame in current direction.
    void moveForward();
    void moveBackward();

  private:
    // Where is this actor (the top left corner) in pixel space?
    int x_;
    int y_;

    // Which direction is this actor facing?
    Direction direction_;
    
    // Speed in pixels/frame.
    int speed_;
    
    int tileSize_;
};

#endif /* actor_h */
