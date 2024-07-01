#include "actor.h"
#include <cassert>

Actor::Actor(int x, int y, int tileSize, int speed)
{
  tileSize_ = tileSize;
  speed_ = speed;
  
  // Place actor at tile (x, y).
  x_ = x * tileSize_;
  y_ = y * tileSize_;
}

int Actor::getX()
{
  return x_;
}

int Actor::getY()
{
  return y_;
}

Direction Actor::getDirection()
{
  return direction_;
}

void Actor::setDirection(Direction direction)
{
  direction_ = direction;
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
