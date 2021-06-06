SUBDIRS = ase cpu emu machine sound sdl

OBJDIRS = $(addprefix ../$(OBJDIR)/, $(SUBDIRS))

TEMP_MAKEFILE = makefile.tmp

all: $(OBJDIRS)

$(OBJDIRS): $(SUBDIRS)

$(SUBDIRS):
	$(MKDIR) ../$(OBJDIR)/$@
	echo OBJDIR:=../../$(OBJDIR)/$@/> $@/$(TEMP_MAKEFILE)
	echo HERE=$@>> $@/$(TEMP_MAKEFILE)
	echo include Makefile>> $@/$(TEMP_MAKEFILE)
	$(MAKE) -C $@ -f $(TEMP_MAKEFILE)
	$(RM) $@/$(TEMP_MAKEFILE)

.PHONY: all $(OBJDIRS) $(SUBDIRS)
