LIBS=/home/veg/devel/libftdi/src/libftdi1.so.2.2.0 
LIBS+=-lusb-1.0 -levdev
LD_FLAGS=-DNDEBUG -O2 -g -rdynamic -Wl,-rpath,/home/veg/devel/libftdi/src $(LIBS)
CFLAGS=-I/usr/include/libevdev-1.0/
OBJS=b.o keymap.o ev.o

all: b ev

b: $(OBJS)
	/usr/bin/cc keymap.o b.o -o b $(LD_FLAGS)

ev: $(OBJS)
	/usr/bin/cc keymap.o ev.o -o ev $(LD_FLAGS)

%.o: %.c
	/usr/bin/cc -O2 -g -DNDEBUG $(CFLAGS) -rdynamic -c $^ -o $@

clean:
	rm *.o b
