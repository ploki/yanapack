Yanapack: Yet Another Nodal Analysis PACKage
============================================

This nodal analysis software is dedicated to electro-mechanical-acoustical equivalent circuit simulation (from a lumped element model).

At this time, only linear analysis in the frequency domain is supported. A simple Gauss-Jordan algorithm is used and no special attention is taken on conditioning the system of equations.

It is work in progress.

This library will be used in a improved version of the loudspeaker graphing calculator ( https://github.com/ploki/Loudspeaker/ )

### References

The science behind this simulation software comes from:
 - Acoustics: Sound Fields and Transducers (Leo L. Beranek, Tim Mellow) ISBN-13: 978-0123914217
 - Electroacoustic modelling of the subwoofer enclosures (Janne Ahonen, Linearteam) http://koti.kapsi.fi/jahonen/


### Elements simulated

Here is a list of passive elements simulated:
 - for conventional electrical simulations
   * Resistors
   * Capacitors
   * Inductors
   * Transformers
   * Gyrators
   * Current sources
   * Tension sources
 - specific to acoustic simulations
   * Pistonic radiator, based on Bessel functions, represents the acoustic impedance of circular pistonic radiators
   * Port impedance of vented enclosures
   * Box impedance of enclosures
   * Free air impedance from the source to the listener

In addition to elementary dipoles, it is possible to create custom sub circuits at will.

Availability
------------

This package is always available from https://github.com/ploki/yanapack

It is distributed under the 3-clause BSD License


Installation
------------

### Prerequisites

GSL is needer in order to build yanapack. You can grab it from ftp.gnu.org/pub/gnu/gsl
or install it using your distribution channel

``` bash
# apt-get install libgsl0-dev
```

### Build the software


``` bash
$ cd yanapack
$ make
$ sudo make install
$ yanapack -h
Yanapack: Yet Another Nodal Analysis PACKage
usage: yanapack -h -n ARG -c ARG -s -f ARG -t ARG -p ARG -F ARG -T ARG
	-h	--help
	-n	--netlist	ARG
	-c	--command	ARG
	-s	--interactive
	-f	--from-frequency	ARG
	-t	--to-frequency	ARG
	-p	--steps-per-decade	ARG
	-F	--from-log-frequency	ARG
	-T	--to-log-frequency	ARG

```

Simulation example
------------------

``` bash
$ ./yanapack -p 3 -F 1 -T 5 -n circuits/Splifftown4000XL.cir
10	     	61.4009337923	61.4008341279	61.400634798	
21.5443469003	73.6382199187	73.6377573139	73.6368320797	
46.4158883361	86.6703029922	86.6681556961	86.6638605731	
100		90.1195363133	90.1095680653	90.0896201165	
215.443469003	88.2440781064	88.1978006968	88.1049982448	
464.158883361	85.6225850622	85.4090506212	84.9767048102	
1000		82.9939913009	82.1068461721	80.2707678201	
2154.43469003	81.1144652732	80.1735093898	79.0467362715	
4641.58883361	83.8185021294	83.1853758654	82.1128859194	
10000		85.6055810208	83.0576454825	76.8973740717	
21544.3469003	85.9705093178	69.9600732923	67.6158439568	
46415.8883361	85.5790604228	42.9950594198	56.9877843479	
100000		83.8266358783	52.6660228355	18.127421656	
```

This command loads the netlist from circuits/Splifftown4000XL.cir
and performs the simulation from 10^1 to 10^5 Hertz with 3 steps per decades.
The result of the simulation is dumped to the standard output

It is also possible to spawn a shell with the simulation just run and issue custom
commands to control the output

``` bash
$ ./yanapack -p 3 -F 1 -T 5 -n circuits/Splifftown4000XL.cir -s
YANAPACK: Yet Another Nodal Analysis PACKage
INFO: running simulation on circuits/Splifftown4000XL.cir
yanapack> F . v1 v0 - IEg / DUP . ARG DEG .
10	      	 7.18383729769	-169.648158946	
21.5443469003	 8.24580233096	-158.534387572	
46.4158883361	 31.5068083514	-177.571739183	
100		 6.05572254378	156.385407005	
215.443469003	 6.17516376084	-174.987686669	
464.158883361	 8.09595523	-167.148304041	
1000		 8.96102021192	-171.981197202	
2154.43469003	 11.5810920751	-175.041192742	
4641.58883361	 9.31120528158	164.36790415	
10000		 7.2895574377	171.04113864	
21544.3469003	 6.87402627954	-179.049579849	
46415.8883361	 7.12122947082	-168.235329689	
100000		 8.47192784203	-155.627338105	
yanapack> 
```

Output is controlled using a forth like language. Here, the output is composed of the frequency, the magnitude and the phase of the impedance of the whole circuit seen from the power amplifier.

[Splifftown4000XL circuit file](https://github.com/ploki/yanapack/blob/master/circuits/Splifftown4000XL.cir)

Simulation files
----------------

This software implements a small subset of what ngspice and gnucap are capable to do.

To start a simulation, first you need a netlist that describ the circuit, here comes the spice-like stuffs. Then, you need to ask yanapack to output what you are looking for, here comes the forth-like stuffs.

### The spice-like stuffs

The netlist is a list of dipole. For each dipole, you specify the magnitude, the connections with each others and other specific parameters

Each dipole is defined on a new line. The first character specifies the type of dipole and each name must be unique for each type of dipole

The dipoles defined are given a magnitude as a function of s where s = j&omega;.

#### Resistor
```
Rxyz x y r
```
 - xyz is the resistor's name
 - x and y are the interconnection nodes
 - r is the value of the resistor in ohm

#### Capacitor
```
Cxyz x y c
```
 - xyz is the capacitor's name
 - x and y are the interconnection nodes
 - c is the value of the capacitor in farad. the impedance is defined as z = 1/(s*c)

#### Inductor
```
Lxyz x y c r
```
 - xyz is the inductor's name
 - x and y are the interconnection nodes
 - r is the series resistor of the actual inductor (0 if ommited)
 - l is the value of the inductor in henry. the impedance is defined as z = s*l

#### Tension source
```
Exyz x y v
Vxyz x y v
```
 - E and V are possible prefix for tension sources
 - xyz is the tension source's name
 - x and y are the interconnection nodes
 - v is the electric potential between x and y (in than order) and is constant at all frequencies

#### Current source
```
Ixyz x y i
```
 - E and V are possible prefix for tension sources
 - xyz is the current source's name
 - x and y are the interconnection nodes
 - i is the electric current crossing the dipole and is constant at all frequencies

#### Ideal Transformer
```
Tlxyz x y i
Trxyz w z j
```
 - l and r specify which side of the transformer is defined. Both sides must be defined.
 - xyz is the transformer's name
 - x and y are the interconnection nodes of the left side of the transformer
 - w and z are the interconnection nodes of the right side of the transformer
 - i and j are the coefficient that represent the windings coupling

#### Gyrator
A gyrator is an hypothetical non reciprocal linear element that inverts the current-voltage characteristic of the circuit between the sides of the device. It is used to pass from impedance to admitance analogy (and the reverse of course)
```
Glxyz x y i
Grxyz w z j
```
 - l and r specify which side of the gyrator is defined. Both sides must be defined.
 - xyz is the gyrator's name
 - x and y are the interconnection nodes of the left side of the gyrator
 - w and z are the interconnection nodes of the right side of the gyrator
 - i and j are the coefficient that represent the gyration resistance


#### Pistonic radiator
```
Zxyz x y surface [ma][ai]
```
 - xyz is the radiator's name
 - x and y are the interconnection nodes of the radiator
 - surface is the surface of the circular piston in square meters
 - [ma][ai] are two charaters to select the domain and the analogy
   * the first set is for the domain selection. 'm' stands for mechanical and 'a' stands for acoustic
   * the second set is for the analogy selection. 'a' stands for admitance and 'i' stands for impedance

The Piston radiation impedance may be found in "Electroacoustic modelling of the subwoofer enclosures" chapter 2.5
 
#### Port impedance
```
Pxyz x y length surface [kK][kK][dr]
```
 - xyz is the port's name
 - x and y are the interconnection nodes of the port
 - surface is the surface of the port in square meters
 - length is the length of the port in meters
 - [kK][kK][dr] are three characters to select the end correction factors and the exactness of the simulation
   * k stands for free air termination
   * K stands for flanged termination
   * d stands for damped simulation (kind of simplified)
   * r stands for resonant simulation (with resonant harmonics)

The Port inertance impedance may be found in "beranek" chapter 4 eq. 4.1, 4.22 and 4.23

#### Box impedance
```
Bxyz x y volume
```
 - xyz is the box's name
 - x and y are the interconnection nodes of the box
 - volume is the volume of the box in cubic meters. The box impedance is defined as z = 1 / (s* volume/( &rho; * c^2 ) )


#### Free air impedance
Completely useless, it pollutes the circuit impedance and hence the tension source load
 
### The forth-like stuffs

### Number formats

Contributing
------------

I would be happy:
 - if you find any interest in this program
 - to make corrections to the code if you find any errors.
 - to merge your contributions
