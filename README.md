# Tickle

Tickle is a multi-platform arcade machine emulator that supports early classics.

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
| 0 | Service |
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

Additionally, Player 1 can use the arrow keys for movement and the spacebar as the main action key.

## How to build on Linux

1. Install the prerequisite [SDL 2.0](https://www.libsdl.org) library:
```
sudo apt-get update
sudo apt-get install libsdl2-2.0
sudo apt-get install libsdl2-dev
```

This should work on Debian, Ubuntu and several other distributions. If it doesn't, check the SDL Wiki on [Installing SDL](https://wiki.libsdl.org/Installation).

2. If necessary, install the development tools:
```
sudo apt-get install build-essential
```

3. Run:
```
make -f Makefile.rpi
```

Executable is placed in the `obj` directory.

## How to build on macOS

1. If necessary, install Xcode Command Line Developer Tools:
```
xcode-select --install
```
If the command line tools are missing, compilation will fail with errors like:
```
xcrun: error: invalid active developer path
```

2. Install the prerequisite [SDL 2.0](https://www.libsdl.org) library with [Homebrew](https://brew.sh/):
```
brew install sdl2
```

Note: SDL's official image for macOS creates a different configuration and will not work with Tickle.

3. Run:
```
make -f Makefile.rpi
```

Executable is placed in the `obj` directory.

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

1. Install [Git for Windows](https://gitforwindows.org/).

Actually the only required component is Git Bash, but they come all bundled together.

2. Install the prerequisite MinGW-w64 toolchain from the [SourceForge website](https://sourceforge.net/projects/mingw-w64/).

A detailed list of installation steps is explained in the guide [Using GCC with MinGW](https://code.visualstudio.com/docs/cpp/config-mingw). Steps 1 and 2 can be skipped.

Another very good guide is [How to Setup SDL2 on Windows for C/C++](https://www.matsson.com/prog/sdl2-mingw-w64-tutorial.php). It covers both MinGW and SDL 2, which is another prerequisite.

3. Install the prerequisite [SDL 2.0](https://www.libsdl.org) library:
- go to the [SDL 2.0 download page](https://www.libsdl.org/download-2.0.php) and download the SDL 2.0 MinGW archive in the Development Libraries section
- extract the downloaded archive to a proper directory (`tar` is installed by MinGW in the previous step but it's also possible to use an unarchiver like 7-Zip), for example:
```
tar -xf C:\Downloads\SDL2-devel-2.0.14-mingw.tar.gz -C c:\
```
Take note of the destination folder, in the above example it would be `c:\SDL2-2.0.14`.

4. Open `Makefile.win` with a text editor and update the `SDL_HOME` variable with the correct value from the previous step. Make sure to replace backslash characters `\` with forward slashes `/`. For example:
```
SDL_HOME = c:/SDL2-2.0.14
```

5. **From a Git Bash shell** run:
```
mingw32-make -f Makefile.win
```
Executable is placed in the `obj` directory.

Note: executable depends on `SDL2.dll`. This library is copied inside `obj` during the build. Make sure to copy both `tickle.exe` and `SDL2.dll` if you want to run the program from another directory.

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
| Driver | Name | Year | Manufacturer | Required resources |
| --- | --- | --- | --- | --- |
| 1942 | 1942 | 1984 | Taito | 1942.zip |
| amidars | Amidar (on Scramble hardware) | 1982 | Konami | amidars.zip, amidar.zip, scramble.zip |
| atlantis | Battle of Atlantis | 1981 | Comsoft | atlantis.zip, scramble.zip |
| blkhole | Black Hole | 1981 | TDS | blkhole.zip, galaxian.zip |
| crush | Crush Roller | 1981 | Kural Samno Electric | crush.zip, puckman.zip |
| eyes | Eyes | 1982 | Rock-Ola | eyes.zip, puckman.zip |
| fantasyu | Fantasy (US) | 1981 | Rock-Ola | fantasyu.zip, fantasy.zip, nibbler.zip |
| frogger | Frogger | 1981 | Konami | frogger.zip |
| galaga | Galaga | 1981 | Namco | galaga.zip |
| galaxian | Galaxian | 1979 | Namco | galaxian.zip |
| gteikoku | Gingateikoku No Gyakushu | 1980 | Irem | gteikoku.zip, uniwars.zip, galaxian.zip |
| jumpshot | Jump Shot | 1985 | Bally Midway | jumpshot.zip, puckman.zip |
| lrescue | Lunar Rescue | 1979 | Taito | lrescue.zip, invaders.zip |
| maketrax | Make Trax | 1981 | Williams Electronics | maketrax.zip, crush.zip, puckman.zip |
| mars | Mars | 1981 | Artic | mars.zip, scramble.zip |
| mooncrst | Moon Cresta | 1980 | Nichibutsu | mooncrst.zip, galaxian.zip |
| mspacman | Ms. Pacman | 1981 | Midway | mspacman.zip, pacman.zip, puckman.zip |
| nrallyx | New Rally X | 1981 | Namco | nrallyx.zip, rallyx.zip |
| nibbler | Nibbler | 1982 | Rock-Ola | nibbler.zip |
| ozmawars | Ozma Wars | 1979 | SNK | ozmawars.zip, invaders.zip |
| pacman | Pacman | 1980 | Midway | pacman.zip, puckman.zip |
| pacplus | Pacman Plus | 1982 | Midway | pacplus.zip, puckman.zip |
| pengo2u | Pengo (set 2 not encrypted) | 1982 | Sega | pengo2u.zip, pengo.zip |
| pbaction | Pinball Action | 1985 | Tehkan | pbaction.zip |
| pooyan | Pooyan | 1982 | Konami | pooyan.zip |
| puckman | Puckman | 1980 | Namco | puckman.zip |
| rallyx | Rally X | 1980 | Namco | rallyx.zip |
| rollingc | Rolling Crash / Moon Base | 1979 | Nichibutsu | rollingc.zip, invaders.zip |
| scramble | Scramble | 1981 | Konami | scramble.zip |
| spaceat2 | Space Attack II | 1980 | Zenitone-Microsec LTD | spaceat2.zip, invaders.zip |
| invaders | Space Invaders | 1979 | Taito | invaders.zip |
| invaddlx | Space Invaders Deluxe | 1979 | Midway | invaddlx.zip, invadpt2.zip, invaders.zip |
| invadpt2 | Space Invaders Part II | 1979 | Taito | invadpt2.zip, invaders.zip |
| theend | The End | 1980 | Konami | theend.zip, scramble.zip |
| uniwars | UniWar S | 1980 | Irem | uniwars.zip, galaxian.zip |
| vanguard | Vanguard | 1981 | SNK | vanguard.zip |
| warofbug | War of the Bugs | 1981 | Armenia | warofbug.zip, galaxian.zip |

## License

Tickle is released under the MIT license, see LICENSE file.

