LIBS=/usr/local/lib/libftdi1.so.2.2.0 
LIBS+=-lusb-1.0 -levdev
LD_FLAGS=-DNDEBUG -O2 -g -rdynamic -Wl,-rpath,/home/veg/devel/libftdi/src $(LIBS)
CFLAGS=-O2 -Wall -I/usr/include/libevdev-1.0/
OBJS=keymap.o qkb.o
CC=gcc

all: qkb

qkb: $(OBJS)
	$(CC) keymap.o qkb.o -o qkb $(LD_FLAGS)

%.o: %.c
	$(CC) -O2 -g -DNDEBUG $(CFLAGS) -rdynamic -c $^ -o $@

clean:
	rm -f *.o qkb
