Yanapack: Yet Another Nodal Analysis PACKage
============================================

This nodal analysis software is dedicated to electro-mechanical-acoustical equivalent circuit simulation (from a lumped element model).

At this time, only linear analysis in the frequency domain is supported. A simple Gauss-Jordan algorithm is used and no special attention is taken on conditioning the system of equations.

It is work in progress.

This library will be used in a improved version of the loudspeaker graphing calculator ( https://github.com/ploki/Loudspeaker/ )

### References

The science behind this simulation software comes from:
 - Acoustics: Sound Fields and Transducers (Leo L. Beranek, Tim Mellow) ISBN-13: 978-0123914217
 - Electroacoustic modelling of the subwoofer enclosures (Janne Ahonen, Linearteam) http://koti.kapsi.fi/jahonen/Audio/Papers/enclosuremodelling.pdf


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
Current source is not an elementary dipole type in yanapack but can be modeled with a tension source and a gyrator using a sub circuit.
```
.subckt current_source v1 v2 current
Eg 1 0 {current}
Glcurrent_source 1 0 1
Rlinkage 0 v2 1T
Grcurrent_source v1 v2 1
.ends

Xxyz x y current_source param: {i TO current}
```
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
 
### The Forth-like stuffs

The embedded Forth-like interpreter is used in different ways inside the circuit file or during the interactive sessions (with the -s parameter)
 - To display simulation results. The script is embedded in the circuit file and all lines starting with ". " (a dot and a space) are considered Forth code
 - To provide parameters to custom sub circuits. after the "param:" keyword inside a block delimited by curly brackets
 - To push values inside the custom sub circuit being instanciated
 - To display simulation results from an interactive session. In this case lines must not start with a dot.

#### Data type

There is only one data type, complex numbers.

The DOT word output the magnitude of the complex number which is on the top of the stack.

#### Arithmetic words
 - __MUL__: 2 3 MUL . => 6
 - __*__: MUL alias 
 - __DIV__: 2 3 DIV . => .6667
 - __/__: DIV alias
 - __ADD__: 2 3 ADD . => 5
 - __+__: ADD alias
 - __SUB__: 2 3 SUB . => -1
 - __-__: SUB ALIAS
 - __POW__: 2 3 POW . => 8
 - __NEG__: 3 NEG . => -3
 - __EXP__: 2 EXP . => 7.38905 (e^2)
 - __e__: EXP alias
 - __SQRT__: 2 SQRT . => 1.4142
 - __LN__: 2 LN . => 0.693147 (ln(2))
 - __LOG__: 2 LOG . => 0.30103 (log10(2))
 - __COS__: 2 COS . => 0.99939 (cos(2))
 - __SIN__: 2 SIN . => 0.03489 (sin(2))
 - __TAN__: 2 TAN . => 0.03492 (tan(2))
 - __ACOS__: .707 ACOS . => 0.78555 (acos(.707))
 - __ASIN__: .707 ASIN . => 0.78525 (asin(.707))
 - __ATAN__: .707 ATAN . => 0.61541 (atan(.707))
 - __ABS__: -1 ABS . I ABS . => 1 1 (abs(-1), abs(i))
 - __ARG__: -1 ARG . I ARG . => 3.14159 1.5708 (arg(-1), arg(i))
 - __IMAG__: -1 IMAG . I IMAG . => 0 1
 - __REAL__: -1 REAL . I REAL . => -1 0

#### Stack manipulation words
 - __DROP__: drop the element on the top of the stack
 - __DUP__: duplicate the element on the top of the stack
 - __SWAP__: swap the element on the top of the stack with the next one
 - __DEPTH__: push on the stack the number of elements in the stack
 - __TO__: pop a value from the stack and assign the value to the word following TO

#### Comparison words
 - __\_LT__: push a non zero value if NOS < TOS
 - __&lt;__: \_LT alias
 - __\_LE__: push a non zero value if NOS <= TOS
 - __&lt;=__: \_LE alias
 - __\_EQ__: push a non zero value if NOS equals TOS
 - __=__: \_EQ alias
 - __==__: \_EQ alias
 - __\_NE__: push a non zero value if NOS is not equel to TOS
 - __&lt;&gt;__: \_NE alias
 - __!=__: \_NE alias
 - __\_GE__: push a non zero value if NOS >= TOS
 - __&gt;=__: \_GE alias
 - __\_GT__: push a non zero value if NOS > TOS
 - __&gt;__: \_GT alias

#### Control words
 - __DOT__: pop an element from the stack and output its magnitude
 - __.__: DOT alias
 - __IF__: jump to the word following the next ELSE word if popped value is 0
 - __ELSE__: jump to the word following the next THEN
 - __THEN__: do nothing
 - __BEGIN__: do nothing
 - __WHILE__: jump to the word following the next REPEAT word if popped value is 0
 - __REPEAT__: jump to the last BEGIN word
 - __UNTIL__: jump to the last begin if popped value is 0
 - __AGAIN__: jump to the last begin
 - __LEAVE__: jump to the word following the next REPEAT/UNTIL/AGAIN
 - __BREAK__: LEAVE alias

#### Mathematical and physics constant words
 - __PI__: push the value of PI
 - __\_RHO__: push the value of the density of the ambiant air
 - __\_C__: push the value of the speed of sound
 - __\_MU__: push the value of the ambiant air viscosity
 - __I__: push the value of the square root of -1

#### Special words
 - __F__: push the current simulation frequency
 - __S__: push the current simulation step
 - __DB__: pop an amplitude and push its dB value (it means it's 20log10(x))
 - __DBSPL__: pop a pression in Pa and push its dB SPL value (20log10(x/(20e-6)))
 - __DEG__: pop an angle in radian push its value in degrees 
 - __PDELAY__: pop a complex value and push the phase delay in regard to its frequency
 - __PREV_STEP__: switch to the last simulation step
 - __&lt;&lt;&lt;__: PREV_STEP alias
 - __NEXT_STEP__: switch to the next simulation step
 - __&gt;&gt;&gt;__: NEXT_STEP alias
 - __PAR__: pop v2 and v1 and push the result of (v1*v2)/(v1+v2)
 - __//__: PAR alias
 - __ANGLE__: pop 2 phases in radian and push the value of the angle between the 2 phases
 - __FREEAIR__: compute the free air impedance for a given listening distance and a solid angle. Pop the solid angle divided by PI in which the radiation happens and then pop the distance to the listener. Finaly, push the computation in the stack
 - __DIRIMP__: compute the free air impedance including the directivity pattern of the circular pistonic driver. Pop the angle, Pop the distance and pop the radiation surface. Finaly, push the computation in the stack

### Number formats

In both Forth context or in the netlist definition, only real floating point numbers are permitted. The numbers are represented as usual and the following are all valid figures
 - 1
 - 1.2
 - .56
 - .68e-3

In addition, there are several suffixes to perform basic conversion or add unit prefixes from the metric system.
 - __f__: * 10^-15
 - __p__: * 10^-12
 - __n__: * 10^-9
 - __u__, __µ__: * 10^-6
 - __m__: * 10^-3
 - __c__: * 10^-2
 - __d__: * 10^-1
 - __k__: * 10^3
 - __M__, __meg__: * 10^6
 - __G__, 10^9
 - __T__, 10^12

Length suffixes
 - __mm__: / 1000
 - __cm__: / 100
 - __dm__: / 10
 - __in__, __"__: * 0.0254
 - __ft__, __'__: * 0.3048
 - __yd__: * 0.9144

Surface suffixes
 - __mm^2__, __mm²__: / 1000000
 - __cm^2__, __cm²__: / 10000
 - __dm^2__, __dm²__: / 100
 - __in^2__, __in²__, __sq.in__: * 0.00064516
 - __ft^2__, __ft²__, __sq.ft__: * 0.092903
 - __yd^2__, __yd²__, __sq.yd__: * 0.836127

Volume suffixes
 - __L__: / 1000
 - __in^3__, __in³__, __cu.in__: * 1.63871e-5
 - __ft^3__, __ft³__, __cu.ft__: * 0.0283168
 - __yd^3__, __yd³__, __cu.yd__: * 0.764555

Angle suffixes
 - __deg__, __°__: * PI/180

### Preprocessor

#### .include
#### .subckt/.ends


Contributing
------------

I would be happy:
 - if you find any interest in this program
 - to make corrections to the code if you find any errors.
 - to merge your contributions
