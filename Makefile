OFILES = simulation_context.o \
	dipole.o \
	netlist.o \
	nodelist.o \
	simulation.o \
	solver.o \
	yana_map.o \
	cirpp.o \
	uforth.o \
	stehfest.o

COMMON_CFLAGS = `gsl-config --cflags` -fPIC -Wall -Werror -I.
RELEASE_CFLAGS = $(COMMON_CFLAGS) -O9 -mfpmath=sse  -fopenmp
DEBUG_CFLAGS = $(COMMON_CFLAGS) -O0 -ggdb3 

.PHONY: all debug install

install all: CFLAGS = $(RELEASE_CFLAGS)
debug:  CFLAGS = $(DEBUG_CFLAGS)
all: binaries
debug: binaries

# -fopenmp

binaries: libyanapack.so yanapack

libyanapack.so: $(OFILES)
	gcc -shared $(OFILES) -o libyanapack.so `gsl-config --libs` -lgomp

yanapack_test: $(OFILES) test.o
	gcc $(OFILES) test.o -o yanapack_test `gsl-config --libs` -lgomp

yanapack: $(OFILES) main.o
	gcc $(OFILES) main.o -o yanapack `gsl-config --libs` -lgomp -ledit

clean:
	rm -f libyanapack.so yanapack_test yanapack $(OFILES) test.o main.o

install: yanapack
	install -d $(DESTDIR)/usr/bin
	install yanapack $(DESTDIR)/usr/bin
