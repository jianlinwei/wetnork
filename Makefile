TARGET = wetnork
PARTICLES = common net
OBJDIR = obj
BINDIR = bin

LIBRARIES = gnutls
LIBRARIES_WITHOUT_PKGCONFIG = -lev

SRC = wetnork.cpp

DEP_SRC = $(SRC)
OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC))

DIRS = $(BINDIR) $(subst ./,,$(sort $(patsubst %,$(OBJDIR)/%,$(dir $(DEP_SRC)))))



all: $(BINDIR)/$(TARGET)

include $(patsubst %,%/Makefile,$(PARTICLES))

DEPFILES = $(DEP_SRC:.cpp=.d)

-include .depend-check
-include $(DEPFILES)

PARTICLE_LIBRARIES = $(patsubst %,-l%,$(PARTICLES))

DEFINES = -DEV_COMPAT3=0
CPPFLAGS = -I include $(DEFINES)
CXXFLAGS += -O2 -ansi -Wall -Wnon-virtual-dtor -pedantic `pkg-config --cflags $(LIBRARIES)`
LDFLAGS += -L $(OBJDIR) `pkg-config --libs $(LIBRARIES)` $(LIBRARIES_WITHOUT_PKGCONFIG) $(PARTICLE_LIBRARIES)



.depend-check:
	@if ! pkg-config --atleast-version=3 gnutls; then echo "gnutls version 3 or newer required"; exit 1; fi

$(BINDIR)/$(TARGET): $(OBJ) $(patsubst %,$(OBJDIR)/lib%.a,$(PARTICLES)) | $(BINDIR)
	$(CXX) -o $@ $(OBJ) $(LDFLAGS)

clean:
	-rm -r $(OBJDIR) $(BINDIR)
   
depclean:
	-rm  $(DEPFILES)

distclean:
	-rm -r $(BINDIR)

$(OBJDIR)/%.o: %.cpp | $(DIRS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(DIRS):
	@mkdir -p $@

%.d: %.cpp
	@echo "Generating dependencies for $<"
	@$(CPP) -MM -MP -MT $(OBJDIR)/$($@:.d=.o) $(CPPFLAGS) $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $@

.PHONY: clean distclean $(DIRS)

