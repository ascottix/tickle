# Tickle

Tickle is an arcade machine emulator that supports early classics.

## How to run

This version does not currently have a user interface (check Tickle 0.94 if you need one) so it must be used from the command line.

Run:

    tickle -list

to get a list of available drivers, or just [check this page](https://ascottix.github.io/tickle/index.html).

Specify a driver name to use it, for example:

    tickle invaders

Tickle expects the necessary ROMs to be placed in a subdirectory named `roms`. The ROM files are the same used in MAME.

ROM files can also be placed in a zip file with the same name as the driver. This is in fact the preferred setup.

For example, when you run `tickle invaders` Tickle will try to read the required files from both `roms` and `roms/invaders.zip`.

### Run fullscreen

By default the program runs in a window. To run fullscreen just add the `-fs` option, for example:

    tickle -fs invaders

### Controls

Tickle supports joysticks and gamepads. It also supports the following keys:

| Key | Function |
| --- | --- |
| 0 | Service Mode |
| 1 | Start Player 1 |
| 2 | Start Player 2 |
| 5 | Insert Coin 1 |
| 6 | Insert Coin 2 |
| A, D, W, S | Player 1 Movement |
| J, L, I, K | Player 2 Movement |
| Left Ctrl | Player 1 Action 1 |
| Z | Player 1 Action 2 |
| X | Player 1 Action 3 |
| C | Player 1 Action 4 |
| E | Player 2 Action 1 |
| R | Player 2 Action 2 |
| T | Player 2 Action 3 |
| G | Player 2 Action 4 |

Arrow keys work as well for movement of Player 1.

## How to build on Linux

TODO

## How to build on macOS

TODO

## How to build on Raspberry Pi

1. Install the prerequisite [SDL 2.0](https://www.libsdl.org) library:
```
sudo apt-get install libsdl2-2.0
sudo apt-get install libsdl2-dev
```
2. Run:
```
make -f Makefile.rpi
```
Executable is placed in the `obj` directory.

## How to build on Windows

TODO

## Emulation

Supported CPUs:
* 6502
* 8080
* Z80

Supported sound chips:
* AY-3-8910
* Namco WSG3
* SN76477
* YM2149

Supported arcades:
| Driver | Name |
| --- | --- |
| 1942 | 1942 | 
| amidars | Amidar (on Scramble hardware) |
| atlantis | Battle of Atlantis |
| blkhole | Black Hole |
| crush | Crush Roller |
| eyes | Eyes |
| fantasyu | Fantasy (US version) |
| frogger | Frogger |
| galaga | Galaga |
| galaxian | Galaxian |
| gteikoku | Gingateikoku No Gyakushu |
| jumpshot | Jump Shot |
| lrescue | Lunar Rescue |
| maketrax | Make Trax |
| mars | Mars |
| mooncrst | Moon Cresta |
| mspacman | Ms. Pacman |
| nrallyx | New Rally X |
| nibbler | Nibbler |
| ozmawars | Ozma Wars |
| pacman | Pacman |
| pacplus | Pacman Plus |
| pengo2u | Pengo | set 2 not encrypted | |
| pbaction | Pinball Action |
| pooyan | Pooyan |
| puckman | Puckman |
| rallyx | Rally X |
| rollingc | Rolling Crash / Moon Base |
| scramble | Scramble |
| spaceat2 | Space Attack II |
| invaders | Space Invaders |
| invaddlx | Space Invaders Deluxe |
| invadpt2 | Space Invaders Part II |
| theend | The End |
| uniwars | UniWar S |
| vanguard | Vanguard |
| warofbug | War of the Bugs |

## License

Tickle is released under the MIT license, see LICENSE file.

