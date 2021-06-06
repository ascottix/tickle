PLAIN_OBJECTS = \
	emu_bitblock.o \
	emu_bitmap.o \
	emu_char_decoder.o \
	emu_crc32.o \
	emu_event2port.o \
	emu_file_iostream.o \
	emu_frame.o \
	emu_info.o \
	emu_input_manager.o \
	emu_iostream.o \
	emu_joystick.o \
	emu_joystick2port.o \
	emu_list.o \
	emu_machine.o \
	emu_memory_iostream.o \
	emu_mixer.o \
	emu_option2port.o \
	emu_palette.o \
	emu_png.o \
	emu_registry.o \
	emu_resource_handler.o \
	emu_resources.o \
	emu_sample.o \
	emu_sample_player.o \
	emu_standard_machine.o \
	emu_string.o \
	emu_tickle_machine.o \
	emu_ui.o \
	emu_zipfile.o

OBJECTS = $(addprefix $(OBJDIR),$(PLAIN_OBJECTS))

$(OBJDIR)%.o : %.cxx
	$(CC) $(CC_FLAGS) -I../zlib -c $< -o $@

EMULIB = $(OBJDIR)../emu.a

target: $(EMULIB)

$(EMULIB): $(OBJECTS)
	$(AR) r $(EMULIB) $(OBJECTS)
