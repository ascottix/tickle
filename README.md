# Tickle

Tickle is an arcade machine emulator.

It supports about 40 machines including 1942, Frogger, Galaga, Galaxian, Moon Cresta, Nibbler, Pacman, Pengo, Pinball Action, Rally X, Scramble, Space Invaders, Vanguard and [many more](https://ascottix.github.io/tickle/index.html).

Hardware:
- 6502 CPU
- 8080 CPU
- Z80 CPU
- AY-3-8910 Sound Generator
- Namco Wave Sound Generator
- SN76477 Complex Sound Generator
- ... and **a lot** of custom hardware and discrete components!

## Notes on the source code

This version is a significant change in the project evolution, as it now uses the [Simple DirectMedia Layer (SDL) library](https://www.libsdl.org/) and no longer provides a native user interface for Windows and Mac OS X.

Using the SDL allows the emulator to run on Linux and Raspbian, which was a main goal of this project (although it wasn't so easy to have it run smoothly on my ancient Raspberry Pi).

Because of the consequent removal of native code, which provided a full featured user interface, the program is now missing the possibility of editing the DIP switch settings for the machines hardware. I will add this back in a portable way as soon as I find a chance to work on this project again. The previous version, which natively supports Windows and Mac OS X, is [archived here](https://github.com/ascottix/tickle094).

Tickle has been compiled and tested on Windows, Linux, Mac OS X and Raspbian. Now this code is quite old and I don't think it will compile cleanly out of the box. However, I made a quick attempt on Windows using MinGW and it wasn't too difficult either. Added:

```
export CC = x86_64-w64-mingw32-g++
```

to the main Makefile, and changed:

```LD = $(CC)
LDFLAGS = -Bstatic -lmingw32 
LIBS = ../obj/machine.a ../obj/cpu.a ../obj/emu.a ../obj/sound.a ../obj/ase.a -lz -lSDL2main -lSDL2

$(TICKLE): $(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)
```

in the sdl/Makefile.

## License

Tickle was previously released with a GPL license. It's now using a MIT license.

See LICENSE file.
