#ifndef timer_h
#define timer_h

#include <stdio.h>

class Timer {

public:
  Timer();
  ~Timer();
  bool isRunning();
  int getTimePassedSecs();
  void start();
  void stop();
  void pause();
  void unpause();
private:
  int startTicks_;
  int pausedTicks_;
  
  bool isPaused_;
  bool isStarted_;
};

#endif /* timer_h */
