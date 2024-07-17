#ifndef timer_h
#define timer_h

#include <stdio.h>

class Timer {

public:
  bool isRunning();
  int getTimePassedSecs();
  void stop();
  void pause();
  void start();
private:
  int count_;
};

#endif /* timer_h */
