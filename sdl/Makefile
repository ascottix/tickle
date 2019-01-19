PLAIN_OBJECTS = \
	sdl_frame.o \
	sdl_main.o \
	fifo.o \
	main.o

OBJECTS = $(addprefix $(OBJDIR),$(PLAIN_OBJECTS))

LD = g++
LDFLAGS = -Bstatic `sdl2-config --cflags --libs`
LIBS = -lSDL2 -lz ../obj/machine.a ../obj/cpu.a ../obj/emu.a ../obj/sound.a ../obj/ase.a

CC_FLAGS += `sdl2-config --cflags --libs` -I..

$(OBJDIR)%.o : %.cxx
	$(CC) $(CC_FLAGS) -c $< -o $@

TICKLE = $(OBJDIR)../tickle

target: $(TICKLE)

$(TICKLE): $(OBJECTS)
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@
