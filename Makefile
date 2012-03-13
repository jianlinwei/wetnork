# main targets and particle libraries
TARGETS = wetnork
PARTICLES = net host common

# default compiler/linker flags
CPPFLAGS += -DEV_COMPAT3=0
CPPFLAGS += -I include

CXXFLAGS += -std=c++11 -fPIC -Wall -Wnon-virtual-dtor -pedantic

LDFLAGS += -pie

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

# external libraries used
LIBRARIES = gnutls
LIBRARIES_WITHOUT_PKGCONFIG = ev

# library version requirements
define LIBRARY_VERSION_CHECK =
	if ! pkg-config --atleast-version=3.0.13 gnutls; then echo "gnutls version 3.0.13 or newer required"; exit 1; fi
endef



###
# here be internals
###

ifdef V
	V = 
else
	V = @
endif

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

ifndef MAKE_RESTARTS
$(shell touch -r Makefile -d yesterday .depend-check)
include .depend-check
else
$(shell rm -f .depend-check)
DEPEND_CHECK_DONE = 1
endif



SRC = $(wildcard *.cpp)

DEP_SRC = $(SRC)

DIRS = $(BINDIR) $(subst ./,,$(sort $(patsubst %,$(OBJDIR)/%,$(dir $(DEP_SRC)))))

PARTICLE_MAKEFILES = $(patsubst %,%/dir.mk,$(PARTICLES))

TARGET_EXECUTABLES = $(patsubst %,$(BINDIR)/%,$(TARGETS))

all: $(TARGET_EXECUTABLES)

.depend-check: Makefile
	@$(LIBRARY_VERSION_CHECK)
	@touch $@

deps:

ifdef DEPEND_CHECK_DONE
-include $(PARTICLE_MAKEFILES)
DEPFILES = $(DEP_SRC:.cpp=.d)
-include $(DEPFILES)
endif

PARTICLE_LIBRARY_NAMES = $(foreach lib,$(PARTICLES),$(call sublib_name,$(lib)))
PARTICLE_LIBRARIES = $(foreach lib,$(PARTICLES),-l$(call submk_name,$(lib)))

CXXFLAGS += `pkg-config --cflags $(LIBRARIES)`
LDFLAGS += -L $(OBJDIR) `pkg-config --libs $(LIBRARIES)` 
LDFLAGS += $(patsubst %,-l%,$(LIBRARIES_WITHOUT_PKGCONFIG)) $(PARTICLE_LIBRARIES)



$(BINDIR)/%: $(OBJDIR)/%.o $(patsubst %,$(OBJDIR)/%,$(PARTICLE_LIBRARY_NAMES)) | $(BINDIR)
	@echo -e "[LD]\t" $@
	$V$(CXX) -o $@ $< $(LDFLAGS)

clean:
	-$(RM) -r $(OBJDIR) $(BINDIR) $(PARTICLE_MAKEFILES)
   
depclean:
	-$(RM)  $(DEPFILES)

distclean:
	-$(RM) -r $(BINDIR)

$(OBJDIR)/%.o: %.cpp Makefile | $(DIRS)
	@echo -e "[CXX]\t" $<
	$V$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(DIRS):
	@mkdir -p $@

%.d: %.cpp Makefile
	@echo -e "[DEP]\t" $<
	$V$(CPP) -MM -MP -MT $(@:.d=.o) $(CPPFLAGS) $< | sed -e 's@^\(.*\)\.o:@\1.d $(OBJDIR)/\1.o:@' > $@

$(PARTICLE_MAKEFILES): Makefile
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
