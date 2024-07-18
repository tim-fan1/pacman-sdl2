#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "level.h"

int main(int argc, char *argv[])
{
  bool success = true;
  Game *game = NULL;
  Level *level = NULL;
  
  // Initialise game.
  game = new Game();
  if (game->getSDLInitSuccess() == false)
  {
    printf("SDL initialisation failed!\n");
    success = false;
  }
  else
  {
    // Make default level.
    level = new Level();

    // Run default level.
    if (!game->run(level)) {
      printf("Failed to run game! Memory allocation error!\n");
    }
  }
  
  // Game is over. Free resources.
  if (game != NULL) delete game;
  if (level != NULL) delete level;
  
  // Exit to shell with approriate exit code.
  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}
