#include "actor.h"
#include <cassert>

Actor::Actor(int tileX, int tileY, int tileSize, int waitingFrames,
             int inBaseTileX, int inBaseTileY, int speed)
{
  tileSize_ = tileSize;
  waitingFrames_ = waitingFrames;
  speed_ = speed;
  
  // Place actor at tile (x, y).
  x_ = tileX * tileSize + (tileSize / 2);
  y_ = tileY * tileSize;
  
  inBaseTileX_ = inBaseTileX;
  inBaseTileY_ = inBaseTileY;
  
  startTileX_ = tileX;
  startTileY_ = tileY;
  
  power_ = 0;
}

Actor::~Actor() {}

int Actor::getX()
{
  return x_;
}

int Actor::getY()
{
  return y_;
}

int Actor::getStartTileX()
{
  return startTileX_;
}

int Actor::getStartTileY()
{
  return startTileY_;
}

void Actor::setTileX(int tileX)
{
  x_ = tileX * tileSize_;
}

void Actor::setTileY(int tileY)
{
  y_ = tileY * tileSize_;
}

Direction Actor::getDirection()
{
  return direction_;
}

void Actor::setDirection(Direction direction)
{
  direction_ = direction;
}

int Actor::getWaitingFrames()
{
  return waitingFrames_;
}

void Actor::setWaitingFrames(int waitingFrames)
{
  waitingFrames_ = waitingFrames;
}

void Actor::setSpeed(int speed)
{
  speed_ = speed;
}

void Actor::turnAround()
{
  if (direction_ == DIRECTION_UP) {
    direction_ = DIRECTION_DOWN;
  } else if (direction_ == DIRECTION_DOWN) {
    direction_ = DIRECTION_UP;
  } else if (direction_ == DIRECTION_LEFT) {
    direction_ = DIRECTION_RIGHT;
  } else if (direction_ == DIRECTION_RIGHT) {
    direction_ = DIRECTION_LEFT;
  }
}

int Actor::getSpotInBaseX()
{
  return (inBaseTileX_ * tileSize_) + (tileSize_ / 2);
}

int Actor::getSpotInBaseY()
{
  return (inBaseTileY_ * tileSize_);
}

bool Actor::getIsFrightened()
{
  return state_ == GHOST_FRIGHTENED;
}

void Actor::setIsFrightened()
{
  state_ = GHOST_FRIGHTENED;
}

bool Actor::getIsEaten()
{
  return state_ == GHOST_EATEN;
}

void Actor::setIsEaten()
{
  state_ = GHOST_EATEN;
}

bool Actor::getIsFindingSpot()
{
  return state_ == GHOST_FINDING_SPOT;
}

void Actor::setIsFindingSpot()
{
  state_ = GHOST_FINDING_SPOT;
}

bool Actor::getIsFindingExit()
{
  return state_ == GHOST_FINDING_EXIT;
}

void Actor::setIsFindingExit()
{
  state_ = GHOST_FINDING_EXIT;
}

bool Actor::getIsChase()
{
  return state_ == GHOST_CHASE;
}

void Actor::setIsChase()
{
  state_ = GHOST_CHASE;
}

bool Actor::getIsScatter()
{
  return state_ == GHOST_SCATTER;
}

void Actor::setIsScatter()
{
  state_ = GHOST_SCATTER;
}

int Actor::getPower()
{
  return power_;
}

void Actor::setPower(int power)
{
  power_ = power;
}

void Actor::moveForward()
{
  if (direction_ == DIRECTION_UP) {
    y_ -= speed_;
  } else if (direction_ == DIRECTION_DOWN) {
    y_ += speed_;
  } else if (direction_ == DIRECTION_LEFT) {
    x_ -= speed_;
  } else if (direction_ == DIRECTION_RIGHT) {
    x_ += speed_;
  }
}

void Actor::moveBackward()
{
  if (direction_ == DIRECTION_UP) {
    y_ += speed_;
  } else if (direction_ == DIRECTION_DOWN) {
    y_ -= speed_;
  } else if (direction_ == DIRECTION_LEFT) {
    x_ += speed_;
  } else if (direction_ == DIRECTION_RIGHT) {
    x_ -= speed_;
  }
}
