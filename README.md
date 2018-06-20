# ltank-sdl Jim Kindley's Laser Tank game remade with SDL 2.0

## Differences from original game

As the source code of the original game is messy and hard to understand, I have
decided to code this remake from scratch and try to keep game logic simple
while retaining overall gameplay of the original game.  As a result, there are
some incompatibilities with the original game and some levels from the original
game are broken.

Here are the major differences from the original game:

* Multiple lasers can be present simultaneously. As a result, anti-tanks will
  fire upon seeing player even if another laser is still in flight.
* 1-tile-long conveyor belts no longer take player to the other end immediately
* Player's laser travels to target instantaneously

There might be other subtle differences. Also see missing features below.

## Missing features

* Ice-related logic is incomplete
* Tunnels
* Recording & playback

## Build instructions

1. Install dependencies: `sdl2`, `sdl2_image`, `sdl2_ttf` and their respective
   development packages
2. Type `make`
3. Run the game: `./ltank`

## Game control

* Press arrow keys to move, spacebar to fire
* Press `u` to undo last move
* Press `r` to restart game
* Press `F9` to toggle editor mode
* Press 'a' to toggle animation

## Screenshot

![Level1: Boot Camp](doc/screenshot1.png?raw=true "Level 1: Boot Camp")
