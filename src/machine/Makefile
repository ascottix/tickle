PLAIN_OBJECTS = \
	1942.o \
	fantasy.o \
	frogger.o \
	galaga.o \
	galaxian.o \
	galaxian_soundboard.o \
	invaders.o \
	namco05.o \
	namco51.o \
	nibbler.o \
	pacman.o \
	pengo.o \
	pinball_action.o \
	pooyan.o \
	rallyx.o \
	scramble.o \
	vanguard.o \
	vanguard_soundboard.o \
	z80_ay3_soundboard.o

OBJECTS = $(addprefix $(OBJDIR),$(PLAIN_OBJECTS))

$(OBJDIR)%.o : %.cxx
	$(CC) $(CC_FLAGS) -I./.. -c $< -o $@

DRIVERLIB = $(OBJDIR)../machine.a

target: $(DRIVERLIB)

$(DRIVERLIB): $(OBJECTS)
	$(AR) r $(DRIVERLIB) $(OBJECTS)
