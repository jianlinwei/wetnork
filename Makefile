include functions.mk

TARGET = wetnork

SRC = wetnork.cpp \
	  SocketAddress.cpp \
	  Exception.cpp \
	  NetworkException.cpp \
	  BadAddress.cpp \
	  BadSend.cpp \
	  Packet.cpp \
	  Socket.cpp \
	  Channel.cpp \
	  Link.cpp \
	  UnreliableUdpChannel.cpp \
	  ReliableUdpChannel.cpp \
	  UdpChannel.cpp \
	  UdpLink.cpp \
	  UdpSocket.cpp \
	  TunDevice.cpp

OBJDIR = obj
BINDIR = bin

DEP = $(SRC:.cpp=.d)
OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC))
DIRS = $(OBJDIR) $(BINDIR)

LIBRARIES = gnutls

LIBRARIES_WITHOUT_PKGCONFIG = -lev

DEFINES = -DEV_COMPAT3=0

CPPFLAGS = 
CXXFLAGS += -O2 -ansi -Wall -Wnon-virtual-dtor -pedantic $(DEFINES) `pkg-config --cflags $(LIBRARIES)`
LDFLAGS += `pkg-config --libs $(LIBRARIES)` $(LIBRARIES_WITHOUT_PKGCONFIG)

all: $(BINDIR)/$(TARGET)

.depend-check:
	@if ! pkg-config --atleast-version=3 gnutls; then echo "gnutls version 3 or newer required"; exit 1; fi

-include .depend-check
-include $(DEP)

$(BINDIR)/$(TARGET): $(OBJ) | $(BINDIR)
	$(CXX) $(LDFLAGS) -o $@ $^

clean:
	-rm -r $(OBJDIR) $(BINDIR) $(DEP)

distclean:
	-rm -r $(BINDIR)

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(DIRS):
	@mkdir -p $@

%.d: %.cpp
	@echo "Generating dependencies for $<"
	$(call make-depend,$<,$(subst .d,.o,$@),$@)

.PHONY: clean distclean $(DIRS)

