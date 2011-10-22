TARGET = wetnork

OBJ = wetnork.o

CFLAGS = -O2 -std=c99 -Wall -Wstrict-prototypes -pedantic

all: $(TARGET)
	@echo -n

-include $(OBJ:.o=.d)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm $(OBJ) $(OBJ:.o=.d) $(TARGET) 

%.d: %.c
	./depend.sh `dirname $*` "$(CC)" $(CFLAGS) $*.c > $@
