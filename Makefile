MKDIR = mkdir -p

AR = ar

OBJDIR = obj

PLATFORM = sdl

SUBDIRS = ase cpu emu machine sound $(PLATFORM)

define make-subdir-target
	$(foreach dir,$(SUBDIRS),
	$(MKDIR) $(OBJDIR)/$(dir)
	echo OBJDIR:=../$(OBJDIR)/$(dir)/> $(dir)/$(TEMP_MAKEFILE)
	echo HERE=$(dir)>> $(dir)/$(TEMP_MAKEFILE)
	echo include Makefile>> $(dir)/$(TEMP_MAKEFILE)
	$(MAKE) -C $(dir) -f $(TEMP_MAKEFILE)
	$(RM) $(dir)/$(TEMP_MAKEFILE)
	)
endef

export CC_FLAGS = -Wall -fno-rtti -fno-exceptions -O2 -static-libgcc

TEMP_MAKEFILE = makefile.tmp

.PHONY: subdirs

subdirs: $(SUBDIRS)
	$(MKDIR) $(OBJDIR)
	$(make-subdir-target)
