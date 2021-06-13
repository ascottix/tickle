PLAIN_OBJECTS = \
	sdl_frame.o \
	sdl_main.o \
	fifo.o \
	main.o

OBJECTS = $(addprefix $(OBJDIR),$(PLAIN_OBJECTS))

LD = $(CC)
LD_FLAGS = -Bstatic $(SDL_LIBS)
LIBS = $(OBJDIR)../machine.a $(OBJDIR)../cpu.a $(OBJDIR)../emu.a $(OBJDIR)../sound.a $(OBJDIR)../ase.a -lz -lm -lstdc++ -lSDL2 -lSDL2main

CC_FLAGS += $(SDL_CFLAGS) -I..

$(OBJDIR)%.o : %.cxx
	$(CC) $(CC_FLAGS) -c $< -o $@

TICKLE = $(OBJDIR)../tickle

target: $(TICKLE)

$(TICKLE): $(OBJECTS)
	$(LD) $(LD_FLAGS) $^ -o $@ $(LIBS)
