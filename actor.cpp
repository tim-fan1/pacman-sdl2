#include "actor.h"
#include <cassert>
#include <cstdio>

Actor::Actor(int tileX, int tileY, int tileSize, Direction initialDirection, int waitingPellets,
             int inBaseTileX, int inBaseTileY, int speed)
{
  tileSize_ = tileSize;
  waitingPellets_ = waitingPellets;
  speed_ = speed;
  direction_ = initialDirection;
  
  // Place actor at tile (x, y).
  x_ = tileX * tileSize + (tileSize / 2);
  y_ = tileY * tileSize;
  
  inBaseTileX_ = inBaseTileX;
  inBaseTileY_ = inBaseTileY;
  
  startTileX_ = tileX;
  startTileY_ = tileY;
  
  targetTileX_ = -1;
  targetTileY_ = -1;
  
  power_ = 0;
  
  state_ = GHOST_NONE;
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

int Actor::getCenterX()
{
  return getX() + (tileSize_ / 2);
}

int Actor::getCenterY()
{
  return getY() + (tileSize_ / 2);
}

int Actor::getTileX()
{
  return getCenterX() / tileSize_;
}

int Actor::getTileY()
{
  return getCenterY() / tileSize_;
}

int Actor::getTileOffsetX()
{
  return getCenterX() % tileSize_;
}

int Actor::getTileOffsetY()
{
  return getCenterY() % tileSize_;
}

void Actor::setTileX(int tileX)
{
  x_ = tileX * tileSize_;
}

void Actor::setTileY(int tileY)
{
  y_ = tileY * tileSize_;
}

int Actor::getTargetTileX()
{
  return targetTileX_;
}

int Actor::getTargetTileY()
{
  return targetTileY_;
}

void Actor::setTargetTileX(int targetTileX)
{
  targetTileX_ = targetTileX;
}

void Actor::setTargetTileY(int targetTileY)
{
  targetTileY_ = targetTileY;
}

Direction Actor::getDirection()
{
  return direction_;
}

void Actor::setDirection(Direction direction)
{
  direction_ = direction;
}

int Actor::getWaitingPellets()
{
  return waitingPellets_;
}

void Actor::setWaitingPellets(int waitingPellets)
{
  waitingPellets_ = waitingPellets;
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

GHOST_STATE Actor::getState()
{
  return state_;
}

void Actor::setState(GHOST_STATE state)
{
  state_ = state;
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

void Actor::printState()
{
  if (state_ == GHOST_NONE) {
    printf("none\n");
  } else if (state_ == GHOST_CHASE) {
    printf("chase\n");
  } else if (state_ == GHOST_SCATTER) {
    printf("scatter\n");
  } else if (state_ == GHOST_FRIGHTENED) {
    printf("frightened\n");
  } else if (state_ == GHOST_EATEN) {
    printf("eaten\n");
  } else if (state_ == GHOST_FINDING_EXIT) {
    printf("exit\n");
  } else if (state_ == GHOST_FINDING_SPOT) {
    printf("spot\n");
  }
}
