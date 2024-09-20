#include <stdio.h>
#include <stdlib.h>

#include "game.h"

int main(int argc, char *argv[])
{
  bool success = true;
  
  // Initialise game.
  Game *game = new Game();
  if (game->getSuccess() == false) {
    printf("Game initialisation failed!\n");
    success = false;
  }
  
  // Run the game.
  if (success && game->run() == false) {
    printf("Error while running game... Check if the game has been successfully initialised!\n");
    success = false;
  }
  
  // Game is over. Free resources.
  if (game != NULL) delete game;
  
  // Exit to shell with approriate exit code.
  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}
