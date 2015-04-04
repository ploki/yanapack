OFILES = src/simulation_context.o \
	src/dipole.o \
	src/netlist.o \
	src/nodelist.o \
	src/simulation.o \
	src/solver.o \
	src/yana_map.o \
	src/cirpp.o \
	src/uforth.o \
	src/stehfest.o

COMMON_CFLAGS = `gsl-config --cflags` -fPIC -Wall -Werror -Isrc
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

yanapack_test: $(OFILES) src/test.o
	gcc $(OFILES) src/test.o -o yanapack_test `gsl-config --libs` -lgomp

yanapack: $(OFILES) src/main.o
	gcc $(OFILES) src/main.o -o yanapack `gsl-config --libs` -lgomp -ledit

clean:
	rm -f libyanapack.so yanapack_test yanapack $(OFILES) src/test.o src/main.o

install: yanapack
	install -d $(DESTDIR)/usr/bin
	install yanapack $(DESTDIR)/usr/bin
