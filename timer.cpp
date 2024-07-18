#include "timer.h"
#include <cmath>
#include <SDL2/SDL.h>

Timer::Timer()
{
  // Reset timer.
  isStarted_ = false;
  isPaused_ = false;
  
  startTicks_ = 0;
  pausedTicks_ = 0;
}

Timer::~Timer() {}

bool Timer::isRunning()
{
  return isStarted_;
}

int Timer::getTimePassedSecs()
{
  return floor((SDL_GetTicks() - startTicks_) / 1000);
}

void Timer::start()
{
  isStarted_ = true;
  isPaused_ = false;
  
  // Start the timer by saving the time when the timer starts.
  startTicks_ = SDL_GetTicks();
  pausedTicks_ = 0;
}

void Timer::stop()
{
  // Reset timer.
  isStarted_ = false;
  isPaused_ = false;
  
  startTicks_ = 0;
  pausedTicks_ = 0;
}

void Timer::pause()
{
  if (isStarted_ && !isPaused_) {
    isPaused_ = true;
    
    // Save how much time has passed since timer started.
    // We can use this to find out what to set startTicks to
    // when we unpause the timer later.
    pausedTicks_ = SDL_GetTicks() - startTicks_;
    startTicks_ = 0;
  }
}

void Timer::unpause()
{
  if (isStarted_ && isPaused_) {
    isPaused_ = false;
    
    // To unpause, let's pretend the timer started
    // pausedTicks milliseconds before now.
    startTicks_ = SDL_GetTicks() - pausedTicks_;
    pausedTicks_ = 0;
  }
}
