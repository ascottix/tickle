PLAIN_OBJECTS = \
	ase.o \
	ase_bandpass_filter.o \
	ase_capacitor_with_switch.o \
	ase_clipper.o \
	ase_inverter.o \
	ase_latch.o \
	ase_lowpass_filter.o \
	ase_multiplexer.o \
	ase_noise.o \
	ase_opamp_noninv1.o \
	ase_switch.o \
	ase_timer555_astable.o \
	ase_timer555_linear_ramp.o \
    ase_triangle_wave_vco1.o \
	complex.o

OBJECTS = $(addprefix $(OBJDIR),$(PLAIN_OBJECTS))

$(OBJDIR)%.o : %.cxx
	$(CC) $(CC_FLAGS) -c $< -o $@

ASELIB = $(OBJDIR)../ase.a

target: $(ASELIB)

$(ASELIB): $(OBJECTS)
	$(AR) r $(ASELIB) $(OBJECTS)
