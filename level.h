// FIXME: Currently is just a wrapper around a std::string.

#ifndef level_h
#define level_h

#include <string>
#include "actor.h"

class Level {
  public:
    /**
     * Calling default constructor results in
     * the default level being instantiated.
     */
    Level();
    ~Level();
    
    /**
     * Returns the level text provided to us on instantiation,
     * which would be the default level's text if the default
     * no-args constructor was called.
     */
    std::string getLevelText();
    int getWidth();
    int getHeight();
    
  private:
    std::string levelText_;
    int width_;
    int height_;
};

#endif /* level_h */
