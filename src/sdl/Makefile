PLAIN_OBJECTS = \
	sdl_frame.o \
	sdl_main.o \
	fifo.o \
	main.o

OBJECTS = $(addprefix $(OBJDIR),$(PLAIN_OBJECTS))

LD = $(CC)
LDFLAGS = -Bstatic `sdl2-config --cflags --libs` 
# LDFLAGS = -Bstatic -L/usr/x86_64-w64-mingw32/sys-root/mingw/lib
# LDFLAGS = -Bstatic -lmingw32 
LIBS = $(OBJDIR)../machine.a $(OBJDIR)../cpu.a $(OBJDIR)../emu.a $(OBJDIR)../sound.a $(OBJDIR)../ase.a -lz -lm -lstdc++ -lSDL2main -lSDL2

CC_FLAGS += `sdl2-config --cflags --libs` -I..

$(OBJDIR)%.o : %.cxx
	$(CC) $(CC_FLAGS) -c $< -o $@

TICKLE = $(OBJDIR)../tickle

target: $(TICKLE)

$(TICKLE): $(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)
