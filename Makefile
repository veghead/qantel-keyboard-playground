all: b

b: b.o keymap.o
	/usr/bin/cc -O2 -g -DNDEBUG -rdynamic b.o keymap.o -o b -Wl,-rpath,/home/veg/devel/libftdi/src /home/veg/devel/libftdi/src/libftdi1.so.2.2.0 -lusb-1.0

%.o: %.c
	/usr/bin/cc -O2 -g -DNDEBUG -rdynamic -c $^ -o $@


