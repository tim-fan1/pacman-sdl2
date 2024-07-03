#include "actor.h"
#include <cassert>

Actor::Actor(int tileX, int tileY, int tileSize, int speed)
{
  tileSize_ = tileSize;
  speed_ = speed;
  
  // Place actor at tile (x, y).
  x_ = tileX * tileSize_;
  y_ = tileY * tileSize_;
  
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

bool Actor::getIsFrightened()
{
  return isFrightened_;
}

void Actor::setIsFrightened(bool isFrightened)
{
  isFrightened_ = isFrightened;
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
