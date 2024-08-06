## Building a PACMAN clone with just SDL2 

SDL2 is a library of useful routines that provides C++ application developers easier access to the complicated hardware components inside computers. For example, using SDL2 provides C++ application developers:

- Access to a computer's graphics processing chip, so that developers can easily create new application windows, and also easily draw graphics onto those windows,
  
- And also access to a computer's keyboard, so that developers can easily find out when keys are pressed on a player's keyboard and when they are let go.

This PACMAN clone game uses mainly these two parts of the SDL2 library to run the game. 

<p align="center">
  <img width="600" src="https://github.com/user-attachments/assets/d4dddc69-3fd8-4d49-b7c9-221595afcca0" alt="Pacman clone gameplay"/>
</p>

## Running the game

This PACMAN clone game runs at 60 frames per second. In other words, the game will re-draw the screen completely 60 times per second, or once every (1/60)th of a second. Deciding what the game will draw on the screen every (1/60)th of a second — every frame — is the bulk of what the game is doing, the bulk of this application's source code.

To handle re-drawing the screen only once every (1/60)th of a second, this application uses a loop like so:

1. Read what the player is pressing down or letting go of on their keyboard right now.
2. Decide what the screen should look like right now, considering also the player's keyboard.
3. Re-draw the screen completely with what we decided the screen should look like right now.
4. Wait for (1/60)th of a second, **minus** how long it took to do steps 1 to 3, so that steps 1 to 4 end up taking as close as possible to (1/60)th of a second.

Every iteration of this loop is called a frame. Since an iteration of this loop takes (1/60)th of a second, 60 iterations of this loop happen every second, which is what is meant by the game running at 60 frames per second.

## Drawing onto the screen

At the very start of the game, so before the player has pressed any keys and before anything should be happening on the screen, the game should just draw the empty maze. So at this time at the very start of the game, we could for every iteration of the loop just do this:

1. Draw this entire picture onto the screen.
2. Wait for (1/60)th of a second, minus how long it took to draw it onto the screen.

<p align="center">
  <img width="600" alt="Default maze before game start" src="https://github.com/user-attachments/assets/7e498773-8534-4355-8cc1-0c9de3f2f120">
</p>

Which would work for the very start of the game. But once the player presses an arrow key, we will have to start moving PACMAN in the direction the player pressed, and then also start moving the ghosts too. And every time PACMAN passes a pellet in the maze, we have to erase the pellet from the picture, and if PACMAN touches a power pellet, we have to change the ghosts in the picture to be a frightened ghost, ..., and so on. Suddenly it's not enough just to draw the same picture onto the screen over and over again.

The solution used in this application is to divide the maze up into 24px by 24px tiles like so:

<p align="center">
  <img width="600" alt="Diagram showing maze divided up into tiles" src="https://github.com/user-attachments/assets/ad74e068-55a6-414f-9847-3283a9140f71">
</p>

So, instead of deciding what the entire screen should look like all at once, we decide what each tile should look like on each frame. For example if PACMAN passes a pellet in the maze, we will just stop drawing a pellet on that tile in the maze, and draw nothing on that tile instead. These are examples of some of the sprites used in drawing the maze, which can be found in the sprite sheet in the source code.

<p align="center">
  <img width="600" alt="Screenshot of the 24px by 24px sprites used by the game" src="https://github.com/user-attachments/assets/ce11e592-522e-44c1-9bc9-9def5e8e8cab">
</p>

And then once we have drawn the maze — all the tiles of the maze — we then draw the ghosts and PACMAN where they should be in the maze. And if the ghosts are frightened we can draw the ghosts as the frightened ghost sprite instead of the normal ghost sprite, and we can change  every 10 frames which PACMAN sprite we use to draw PACMAN to make PACMAN look like they are chomping on the pellets.
