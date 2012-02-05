# main targets and particle libraries
TARGETS = wetnork
PARTICLES = net host common

# default values for internal variables
ifeq "$(origin CXX)" "default"
	CXX = clang++
endif
ifndef OBJDIR
	OBJDIR = obj
endif
ifndef BINDIR
	BINDIR = bin
endif
ifdef V
	V = 
else
	V = @
endif

# external libraries used
LIBRARIES = gnutls
LIBRARIES_WITHOUT_PKGCONFIG = ev



###
# here be internals
###

# argument 1: makefile name or directory name of particle root
# result: cleaned name of particle
define submk_name =
$(subst /,_,$(subst _,__,$(1:/dir.mk=)))
endef

# argument 1: makefile name or directory name of particle root
# result: particles library filename
define sublib_name =
lib$(call submk_name,$1).a
endef

SRC = $(wildcard *.cpp)

DEP_SRC = $(SRC)
OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC))

DIRS = $(BINDIR) $(subst ./,,$(sort $(patsubst %,$(OBJDIR)/%,$(dir $(DEP_SRC)))))

PARTICLE_MAKEFILES = $(patsubst %,%/dir.mk,$(PARTICLES))

TARGET_EXECUTABLES = $(patsubst %,$(BINDIR)/%,$(TARGETS))

all: $(TARGET_EXECUTABLES)

deps:

-include $(PARTICLE_MAKEFILES)

DEPFILES = $(DEP_SRC:.cpp=.d)

-include .depend-check
-include $(DEPFILES)

PARTICLE_LIBRARY_NAMES = $(foreach lib,$(PARTICLES),$(call sublib_name,$(lib)))
PARTICLE_LIBRARIES = $(foreach lib,$(PARTICLES),-l$(call submk_name,$(lib)))

DEFINES = -DEV_COMPAT3=0
CPPFLAGS = -I include $(DEFINES)
CXXFLAGS += -std=c++11 -fPIC -Wall -Wnon-virtual-dtor -pedantic `pkg-config --cflags $(LIBRARIES)`
LDFLAGS += -L $(OBJDIR) `pkg-config --libs $(LIBRARIES)` 
LDFLAGS += $(patsubst %,-l%,$(LIBRARIES_WITHOUT_PKGCONFIG)) $(PARTICLE_LIBRARIES) -pie



.depend-check:
	@if ! pkg-config --atleast-version=3 gnutls; then echo "gnutls version 3 or newer required"; exit 1; fi

$(TARGET_EXECUTABLES): $(OBJ) $(patsubst %,$(OBJDIR)/%,$(PARTICLE_LIBRARY_NAMES)) | $(BINDIR)
	@echo -e "[LD]\t" $@
	$V$(CXX) -o $@ $(OBJ) $(LDFLAGS)

clean:
	-$(RM) -r $(OBJDIR) $(BINDIR) $(PARTICLE_MAKEFILES)
   
depclean:
	-$(RM)  $(DEPFILES)

distclean:
	-$(RM) -r $(BINDIR)

$(OBJDIR)/%.o: %.cpp | $(DIRS)
	@echo -e "[CXX]\t" $<
	$V$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(DIRS):
	@mkdir -p $@

%.d: %.cpp
	@echo -e "[DEP]\t" $<
	$V$(CPP) -MM -MP -MT $(@:.d=.o) $(CPPFLAGS) $< | sed -e 's@^\(.*\)\.o:@\1.d $(OBJDIR)/\1.o:@' > $@

$(PARTICLE_MAKEFILES):
	@echo -e "[GEN]\t" $@
	$(call generate_subdir_makefile,$@)

.PHONY: clean distclean $(DIRS) deps



define generate_subdir_makefile =
	@echo 'DIRS += $$(OBJDIR)/$(1:/dir.mk=)' > $@ 
	@echo >> $@
	@echo '$(call submk_name,$1)_SRC = $$(wildcard $(call submk_name,$1)/*.cpp)' >> $@
	@echo >> $@
	@echo '$(call submk_name,$1)_OBJ = $$(patsubst %.cpp,$$(OBJDIR)/%.o,$$($(call submk_name,$1)_SRC))' >> $@
	@echo >> $@
	@echo 'DEP_SRC += $$($(call submk_name,$1)_SRC)' >> $@
	@echo >> $@
	@echo '$$(OBJDIR)/$(call sublib_name,$1): $$($(call submk_name,$1)_OBJ)' >> $@
	@echo '	@echo -e "[AR]\t" $$@' >> $@
	@echo '	$$V$$(AR) -rcs $$@ $$^' >> $@
endef
