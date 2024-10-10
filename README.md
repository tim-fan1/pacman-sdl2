## Building a PACMAN clone with just SDL2 

<p align="center">
  <img width="600" src="https://github.com/user-attachments/assets/d4dddc69-3fd8-4d49-b7c9-221595afcca0" alt="Pacman clone gameplay"/>
</p>

SDL2 is a library of useful routines that provides C++ application developers easier access to the complicated hardware components inside computers. For example, using SDL2 provides C++ application developers:

- Access to a computer's graphics processing chip, so that developers can easily create new application windows, and also easily draw graphics onto those windows,
  
- And also access to a computer's keyboard, so that developers can easily find out when keys are pressed on a player's keyboard and when they are let go.

This PACMAN clone game uses mainly these two parts of the SDL2 library to run the game. 

## Running the game

This PACMAN clone game runs at 60 frames per second. In other words, the game will re-draw the screen completely — will empty out the screen and re-draw everything completely! — 60 times per second, or once every (1/60)th of a second. Deciding what the game will draw on the screen every (1/60)th of a second is the bulk of what the game is doing, the bulk of this application's source code.

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

Which would work for the very start of the game. But once the player presses an arrow key, we will have to start moving PACMAN in the picture in the direction the player pressed, and then also start moving the ghosts in the picture too. And then every time PACMAN passes a pellet in the maze, we have to erase the pellet from the picture, and if PACMAN touches a power pellet, we have to change the ghosts in the picture to be a frightened ghost, ..., and so on. Suddenly it's not enough just to draw the same picture onto the screen over and over again.

The solution used in this application is to divide the maze up into 24px by 24px tiles like so:

<p align="center">
  <img width="600" alt="Diagram showing maze divided up into tiles" src="https://github.com/user-attachments/assets/ad74e068-55a6-414f-9847-3283a9140f71">
</p>

So, instead of deciding what the entire screen should look like all at once, we decide what each tile should look like individually on each frame. So that if PACMAN passes a pellet in the maze, all the game needs to do is just stop drawing a pellet on that tile in the maze, and draw nothing on that tile instead. 

And then once we have drawn the maze — all the tiles of the maze — we then draw the ghosts and PACMAN where they should be in the maze. And if the ghosts are frightened we can draw the ghosts as the frightened ghost sprite instead of the normal ghost sprite, and we can change  every 10 frames which PACMAN sprite we use to draw PACMAN to make PACMAN look like they are chomping on the pellets. Having our screen be made up of tiles makes deciding what to draw on each frame considerably easier. 

Internally this grid of tiles is stored as a simple 2-D array of 28 by 36 tiles, where each element of this array records what kind of tile should be drawn at the corresponding tile on the computer screen. So if ``board[3][6]`` is a ``TILE_WALL``, we know that when we are re-drawing the screen, the 3rd tile from the left and 6th tile from the top of the screen should be drawn as a wall tile.

Of course PACMAN and the ghosts work a bit differently, where sometimes they are exactly in just one tile, but usually they are overlapping with another adjacent tile, and so we can't just store their positions in the same 28 by 36 2-D array. Instead, what this application does is simply record which pixel the top-left corner of PACMAN should be drawn at, and the same for the ghosts as well. In other words, in order to find out where PACMAN and the ghosts are currently in the maze, we just have to look at our record of where on the computer screen PACMAN and the ghosts were previously drawn at.

For example, if PACMAN was drawn at ``(200, 400)`` on the previous frame, and we want to move PACMAN 5 pixels to the left on this frame, then we will just draw PACMAN at ``(195, 400)`` on this frame, and record that that is where we drew PACMAN.      

<p align="center">
  <img width="200" alt="Diagram showing how the ghosts are usually spread across two overlapping tiles, and only sometimes exactly in just one tile." src="https://github.com/user-attachments/assets/2c3f236a-f6a2-4a54-8748-92848f5d990c">
</p>

## Building the game

With this set-up in mind the game was fairly easy to build. 

Notable decisions include:

- Making the wall tile sprites as appearing as being only a line with a width of 2 pixels or so, when in the game's code walls are actually treated as much larger 24 pixel by 24 pixel squares. The wall tile sprites appearing thinner than the walls actually are, allows us to draw PACMAN and the ghost sprites as being much bigger than PACMAN and the ghosts actually are in the game's code. In the game's code, PACMAN and the ghosts are — like the walls — also treated as being 24 pixel by 24 pixel squares, but they are drawn as 48 pixel by 48 pixel squares.

<p align="center">
  <img width="600" alt="Screenshot of spritesheet showing how wall tile sprites appear as being just a thin line, when really the entire tile is treated as a wall in the game's code." src="https://github.com/user-attachments/assets/ce11e592-522e-44c1-9bc9-9def5e8e8cab">
</p>

- Implementing a input buffer, so that on frames when the game is not reading keypresses from the player's keyboard, even though the player is actually already holding down a key on the keyboard, the game can take from this input buffer to find out what key the player is holding down on the keyboard. 

  Without an input buffer, players would have to press down the arrow key at the exact right moment in order to turn exactly around the corner ahead of them. With an input buffer, players can start holding down the arrow key in advance before reaching the corner, and as long as the key is still being held when they reach the corner, they will successfully turn the corner. Notice in this gif how the player starts holding the up arrow key well in advance before reaching the corner, so that when they arrive to the corner they successfully turn left going upwards.
  
<p align="center">
  <img width="400" alt="Gif showing how input buffering works in gameplay." src="https://github.com/user-attachments/assets/02357312-9be3-405c-a602-4a031b07e394">
</p>
  
Remaining challenges include:

- How to implement variable speeds for PACMAN and ghosts? For example, it would be nice if once PACMAN eats a power pellet, that frightened ghosts start moving slower than they usually do, so that PACMAN can easily catch up to frightened ghosts and send them to their home base.

- Implementing sound effects. This should be somewhat easily doable, the main challenge being creating the sound effects. Interacting with a computer's audio system is also a part of the SDL2 library, so we could use SDL2 to play and pause the sound effects we want at any moment during the game. 

- Implementing score counting. Without variable speeds for PACMAN and ghosts it is difficult to send frightened ghosts back to base, which is one of the main ways to gain points in the original PACMAN game aside from collecting pellets. So for now the goal of this clone is just to collect all pellets while avoiding all ghosts. 

- Implementing multiple levels. In the original PACMAN game, new levels beyond the first level have the same maze as the first level, so the only differences are things like what speed PACMAN and the ghosts are, and how long a power pellet lasts. Without variable speeds for PACMAN and ghosts it would be difficult to implement interesting multiple levels.

## Reflection

The remaining challenges mainly hinge on being able to successfully vary the speeds of PACMAN and the ghosts. When starting this project, this was not something that I had accounted for, and so the codebase ended up relying heavily on a guarantee that the ghosts and PACMAN will always move at a constant speed no matter what state they are in. Implementing variable speeds would require a significant re-factoring of the codebase, which is not achievable for me right now, but would be something I am interested in doing in the future.  
