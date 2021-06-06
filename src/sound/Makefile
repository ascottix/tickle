PLAIN_OBJECTS = \
	ay-3-8910.o \
	namcowsg3.o \
	rcfilter.o \
	sn76477.o \
	ym2149.o \
	waveform.o

OBJECTS = $(addprefix $(OBJDIR),$(PLAIN_OBJECTS))

$(OBJDIR)%.o : %.cxx
	$(CC) $(CC_FLAGS) -c $< -o $@

SOUNDLIB = $(OBJDIR)../sound.a

target: $(SOUNDLIB)

$(SOUNDLIB): $(OBJECTS)
	$(AR) r $(SOUNDLIB) $(OBJECTS)
