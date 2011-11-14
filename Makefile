TARGET = wetnork

OBJ = wetnork.o \
	  network.o \
	  tun.o

LIBRARIES = libconfig \
			gnutls

LIBRARIES_WITHOUT_PKGCONFIG = -lev -lboost_signals

DEFINES = -DEV_COMPAT3=0

CXXFLAGS += -O2 -ansi -Wall -pedantic $(DEFINES) `pkg-config --cflags $(LIBRARIES)`
LDFLAGS += `pkg-config --libs $(LIBRARIES)` $(LIBRARIES_WITHOUT_PKGCONFIG)

all: $(TARGET)
	@echo -n

.depend-check:
	@if ! pkg-config --atleast-version=1.4 libconfig; then echo "libconfig version 1.4 or newer required"; exit 1; fi
	@if ! pkg-config --atleast-version=3 gnutls; then echo "gnutls version 3 or new required"; exit 1; fi
	@touch $@

-include .depend-check
-include $(OBJ:.o=.d)

$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

clean:
	-rm $(OBJ) $(OBJ:.o=.d) $(TARGET) .depend-check

%.d: %.cpp
	./depend.sh `dirname $*` "$(CXX)" $(CXXFLAGS) $*.cpp > $@

.PHONY: clean
