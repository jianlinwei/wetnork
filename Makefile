TARGET = wetnork

OBJ = wetnork.o \
	  tun.o

LIBRARIES = libconfig \
			gnutls

CFLAGS += -O2 -std=c99 -Wall -Wstrict-prototypes -pedantic `pkg-config --cflags $(LIBRARIES)`
LDFLAGS += `pkg-config --libs $(LIBRARIES)`

all: $(TARGET)
	@echo -n

.depend-check:
	@if ! pkg-config --atleast-version=1.4 libconfig; then echo "libconfig version 1.4 or newer required"; exit 1; fi
	@if ! pkg-config --atleast-version=3 gnutls; then echo "gnutls version 3 or new required"; exit 1; fi
	@touch $@

-include .depend-check
-include $(OBJ:.o=.d)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	-rm $(OBJ) $(OBJ:.o=.d) $(TARGET) .depend-check

%.d: %.c
	./depend.sh `dirname $*` "$(CC)" $(CFLAGS) $*.c > $@

.PHONY: clean
