# main targets and particle libraries
TARGETS := wetnork
PARTICLES := net host common

# default compiler/linker flags
CPPFLAGS += -std=c++11 -DEV_COMPAT3=0 -I include

CXXFLAGS += -std=c++11 -fPIE -Wall -Wnon-virtual-dtor -pedantic

LDFLAGS += -pie

ifdef RELEASE
	CXXFLAGS += -O2
else
	CXXFLAGS += -g
	LDFLAGS += -g
endif

# default values for internal variables
CXX := clang++
CPP := $(CXX) -E
SED := sed
FIND := find
ifndef OBJDIR
	OBJDIR = obj
endif
ifndef BINDIR
	BINDIR = bin
endif

# external libraries used
LIBRARIES := gnutls libnl-3.0 libnl-route-3.0
LIBRARIES_WITHOUT_PKGCONFIG := ev

# library version requirements
define LIBRARY_VERSION_CHECK =
	if ! pkg-config --atleast-version=3.0.15 gnutls; then echo "gnutls version 3.0.15 or newer required"; exit 1; fi
	if ! pkg-config --atleast-version=3.2.7 libnl-3.0; then echo "libnl version 3.2.7 or newer required"; exit 1; fi
	if ! pkg-config --atleast-version=3.2.7 libnl-route-3.0; then echo "libnl-route version 3.2.7 or newer required"; exit 1; fi
endef



###
# here be internals
###

ifdef V
override V :=
else
V := @
endif

# result: cleaned name of particle
# argument 1: makefile name or directory name of particle root
define submk_name =
$(subst /,_,$(subst _,__,$(1:/dir.mk=)))
endef

# result: particles library filename
# argument 1: makefile name or directory name of particle root
define sublib_name =
lib$(call submk_name,$1).a
endef

ifndef MAKE_RESTARTS
$(shell touch -r Makefile -d yesterday .depend-check)
include .depend-check
else
$(shell $(RM) .depend-check)
DEPEND_CHECK_DONE := 1
endif



SRC := $(wildcard *.cpp)

DEP_SRC := $(SRC)

DIRS := $(BINDIR) $(subst ./,,$(sort $(patsubst %,$(OBJDIR)/%,$(dir $(DEP_SRC)))))

PARTICLE_MAKEFILES := $(patsubst %,%/dir.mk,$(PARTICLES))

TARGET_EXECUTABLES := $(patsubst %,$(BINDIR)/%,$(TARGETS))

all: $(TARGET_EXECUTABLES)

.depend-check: Makefile
	@$(LIBRARY_VERSION_CHECK)
	@touch $@

deps:

ifdef DEPEND_CHECK_DONE
-include $(PARTICLE_MAKEFILES)
-include $(patsubst %.cpp,$(OBJDIR)/%.o.d,$(DEP_SRC))
endif

PARTICLE_LIBRARY_NAMES := $(foreach lib,$(PARTICLES),$(call sublib_name,$(lib)))
PARTICLE_LIBRARIES := $(foreach lib,$(PARTICLES),-l$(call submk_name,$(lib)))

CXXFLAGS += `pkg-config --cflags $(LIBRARIES)`
LDFLAGS += -L $(OBJDIR) `pkg-config --libs $(LIBRARIES)` 
LDFLAGS += $(patsubst %,-l%,$(LIBRARIES_WITHOUT_PKGCONFIG)) $(PARTICLE_LIBRARIES)



$(BINDIR)/%: $(OBJDIR)/%.o $(patsubst %,$(OBJDIR)/%,$(PARTICLE_LIBRARY_NAMES)) | $(BINDIR)
	@echo -e "[LD]\t" $@
	$V$(CXX) -o $@ $< $(LDFLAGS)

.SECONDARY: $(patsubst %,$(OBJDIR)/%.o,$(TARGETS))

clean:
	-$(RM) -r $(OBJDIR) $(BINDIR) $(PARTICLE_MAKEFILES)
   
depclean:
	-$(FIND) $(OBJDIR) -iname '*.d' -delete

distclean:
	-$(RM) -r $(BINDIR)

$(OBJDIR)/%.o: %.cpp Makefile | $(DIRS)
	@echo -e "[CXX]\t" $<
	$V$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<
	$(call generate_depfile,$<,$@)

$(DIRS):
	@mkdir -p $@

$(PARTICLE_MAKEFILES): Makefile
	@echo -e "[GEN]\t" $@
	$(call generate_subdir_makefile,$@)

html-doc:
	$Vdoxygen doc/doxy.cfg

.PHONY: clean distclean depclean $(DIRS) deps all html-doc


# result: shell command to create a dependency file
# argument 1: input source file
# argument 2: output object file
define generate_depfile =
	$V$(CPP) -MM -MP -MT $2 $(CPPFLAGS) $1 > $2.d \
		&& $(SED) -e 's@^\(.*\)\.o:@\1.d \1.o:@' -i $2.d
endef

# result: shell commands to create a particle Makefile
# argument 1: directory
define generate_subdir_makefile =
	@echo 'DIRS += $$(OBJDIR)/$(1:/dir.mk=)' > $1 
	@echo >> $1
	@echo '$(call submk_name,$1)_SRC := $$(wildcard $(call submk_name,$1)/*.cpp)' >> $1
	@echo >> $1
	@echo '$(call submk_name,$1)_OBJ := $$(patsubst %.cpp,$$(OBJDIR)/%.o,$$($(call submk_name,$1)_SRC))' >> $1
	@echo >> $1
	@echo 'DEP_SRC += $$($(call submk_name,$1)_SRC)' >> $1
	@echo >> $1
	@echo '$$(OBJDIR)/$(call sublib_name,$1): $$($(call submk_name,$1)_OBJ)' >> $1
	@echo '	@echo -e "[AR]\t" $$@' >> $1
	@echo '	$$V$$(AR) -rcs $$@ $$^' >> $1
endef
