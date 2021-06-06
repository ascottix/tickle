N6502_OBJECTS = n6502.o n6502opc.o n6502sub.o
I8080_OBJECTS = i8080.o i8080opc.o i8080sub.o
Z80_OBJECTS = z80.o z80op1.o z80op2cb.o z80op2dd.o z80op2ed.o z80op2fd.o z80op3xycb.o z80sub.o

PLAIN_OBJECTS = $(N6502_OBJECTS) $(I8080_OBJECTS) $(Z80_OBJECTS)

OBJECTS = $(addprefix $(OBJDIR),$(PLAIN_OBJECTS))

$(OBJDIR)%.o : %.cxx
	$(CC) $(CC_FLAGS) -c $< -o $@

CPULIB = $(OBJDIR)../cpu.a

target: $(CPULIB)

$(CPULIB): $(OBJECTS)
	$(AR) r $(CPULIB) $(OBJECTS)
