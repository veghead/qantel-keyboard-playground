all: b

b: b.c
	/usr/bin/cc -O2 -g -DNDEBUG -rdynamic b.c -o b  -Wl,-rpath,/home/veg/devel/libftdi/src /home/veg/devel/libftdi/src/libftdi1.so.2.2.0 -lusb-1.0


