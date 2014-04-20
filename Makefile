OFILES = simulation_context.o \
	dipole.o \
	netlist.o \
	nodelist.o \
	simulation.o \
	solver.o \
	yana_map.o \
	stehfest.o


CFLAGS= -fPIC -Wall -Werror -I. -O0 -ggdb3 `gsl-config --cflags`

# -fopenmp

all: libyanapack.so yanapack

libyanapack.so: $(OFILES)
	gcc -shared $(OFILES) -o libyanapack.so `gsl-config --libs` -lgomp

yanapack: $(OFILES) main.o
	gcc $(OFILES) main.o -o yanapack `gsl-config --libs` -lgomp

clean:
	rm -f libyanapack.so yanapack $(OFILES) main.o
