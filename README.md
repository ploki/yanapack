[![Build Status](https://scan.coverity.com/projects/10036/badge.svg)](https://scan.coverity.com/projects/10036)

Yanapack: Yet Another Nodal Analysis PACKage
============================================

This dedicated nodal analysis software is designed for simulating
electro-mechanical-acoustical equivalent circuits based on a lumped
element model. It allows users to analyze and simulate the behavior
of complex systems by representing them using a circuit-based approach,
incorporating electrical, mechanical, and acoustic elements into the simulation.

The simulation primarily operates in the frequency domain, which is the default domain
for the simulation results. However, by utilizing the `-i` option, users can obtain the
simulation results in the time domain instead. This option also provides the flexibility
to select different types of excitations, including unit impulse, unit step, square waves,
or sine waves of arbitrary frequencies. By enabling the `-i` option, users can analyze and
visualize the system's response over time with various input signals.


### References

The scientific principles underpinning this simulation software are derived from:
 - Acoustics: Sound Fields and Transducers (Leo L. Beranek, Tim Mellow) ISBN-13: 978-0123914217
 - Electroacoustic modelling of the subwoofer enclosures (Janne Ahonen, Linearteam) http://koti.kapsi.fi/jahonen/Audio/Papers/enclosuremodelling.pdf


### Elements simulated

The simulation software incorporates the simulation of various passive elements, categorized as follows:

 - For conventional electrical simulations:
   * Resistors
   * Capacitors
   * Inductors
   * Transformers
   * Gyrators
   * Current sources
   * Voltage sources

 - Specific to acoustic simulations:
   * Pistonic radiator: Represents the acoustic impedance of circular pistonic radiators, utilizing Bessel functions.
   * Port impedance: Models the acoustic impedance of vented enclosures.
   * Box impedance: Represents the impedance characteristics of enclosures.
   * Free air impedance: Accounts for the acoustic impedance from the sound source to the listener.

Additionally, the software allows for the creation of custom sub-circuits, offering
flexibility in designing and incorporating specialized circuit elements as needed.

Availability
------------

This package is always available from https://github.com/ploki/yanapack

It is distributed under the 3-clause BSD License


Installation
------------

### Prerequisites

GSL is needed in order to build yanapack. You can grab it from ftp.gnu.org/pub/gnu/gsl
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
usage: ./yanapack -h -n ARG -c ARG -s -f ARG -t ARG -p ARG -i ARG
	-h	--help
	-n	--netlist	ARG
	-c	--command	ARG
	-s	--interactive
	-f	--from-frequency	ARG
	-t	--to-frequency	ARG
	-p	--steps-per-decade	ARG
	-i	--impulse	ARG
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

The provided command loads the netlist from the file "circuits/Splifftown4000XL.cir"
and conducts a simulation across the frequency range of 10^1 to 10^5 Hertz, with 3
steps per decade. The simulation results are outputted to the standard output.

Additionally, the software offers the capability to spawn a REPL, allowing users to
interact with the simulation that was just executed. This enables users to issue
custom formulas or commands to control and manipulate the output according to
their specific requirements.


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

Simulation domain
--------------------
This software primarily focuses on frequency domain simulation due
to the limitations of representing the pistonic radiator accurately
in the time domain. As a result, all computations and analyses are
carried out in the frequency domain. However, despite this emphasis,
it is still feasible to compute a response in the time domain for a
given circuit.

To choose between the frequency and time domains, you can specify either
the `-p` or `-i` option on the command line. Using the `-p` option indicates a
preference for the frequency domain, while the `-i` option indicates a
desire to obtain results in the time domain. This allows users to
select the appropriate domain for their specific analysis needs when
running the simulation.

#### Frequency domain
When the `-p` parameter is provided or neither `-p` nor `-i` is specified,
the simulation result is displayed in the frequency domain. In this
scenario, the arguments for the `-f` and `-t` options are interpreted as
the base 10 logarithm of the lower and upper bounds of the frequencies
of interest.

By using the `-f` option followed by a numerical value, you define the base 10
logarithm of the lower frequency bound. Similarly, the `-t` option followed
by a numerical value sets the base 10 logarithm of the upper frequency bound.
This allows you to specify the frequency range over which you want to observe
the simulation results in the frequency domain.

#### Time domain

When the `-i` parameter is utilized, the simulation result in the frequency domain
is extrapolated to the given excitation, determined by an integer argument N
with the following interpretations:
 - If N is greater than 0: The excitation will be a square waveform with a frequency of N Hz.
 - If N is 0: A unit impulse is used as the excitation.
 - If N is -1: A step response is computed.
 - If N is less than -1: The excitation is a pulsation of -exp(i·2·π·N).

When the `-i` parameter is used, the interpretation of the `-f` and `-t` parameters changes.
Instead of representing the base 10 logarithm of the frequency bounds, they are
interpreted differently. Specifically:
 - When using the `-i` parameter, set the argument for the `-f` parameter to 0. This indicates that the simulation will be performed from 0 Hz (e.g. DC).
 - The argument for the `-t` parameter should be set to twice the Nyquist frequency,
   such as 40ish kHz for audio applications. This represents the upper frequency bound.

Simulation files
----------------

This software pays homage to well-known packages like ngspice and gnucap while
having its own idiosyncrasies, serving a specific purpose for its creator, I
like it that way, and I find it very useful. However, at this point, I digress
from the topic at hand.

Similar to Spice, circuit descriptions in this software are provided in a file
known as a netlist. However, unlike Spice, this software offers a unique
feature where the output of the simulation can be customized using a Forth-like
language. This allows users to program and manipulate the simulation results
according to their specific needs and requirements, providing a good level of
flexibility and customization.

### Netlist description

The netlist comprises a list of dipoles, where each dipole is described by its magnitude, connections to other dipoles, and additional parameters specific to the dipole.

Each dipole is defined on a separate line in the netlist. The type of dipole is denoted by the first character, and it is essential for each dipole name to be unique within its dipole type.


#### Resistor
```
Rabc x y r
```
 - abc is the resistor's name
 - x and y are the interconnection nodes
 - r is the value of the resistor in ohm

#### Capacitor
```
Cabc x y c
```
 - abc is the capacitor's name
 - x and y are the interconnection nodes
 - c is the value of the capacitor in farad. The impedance is defined as z = 1/(s*c)

#### Inductor
```
Labc x y l r
```
 - abc is the inductor's name
 - x and y are the interconnection nodes
 - l is the value of the inductor in henry. The impedance is defined as z = s*l
 - r is the series resistor of the actual inductor (0 if ommited)

#### Tension source
```
Eabc x y v
Vabc x y v
```
 - E and V are possible prefix for tension sources
 - abc is the tension source's name
 - x and y are the interconnection nodes
 - v is the electric potential between x and y (in than order) and is constant at all frequencies

Note that output impedance is zero.

#### Current source
Although a current source is not considered an elementary dipole type in yanapack, it can be simulated using a sub circuit consisting of a tension source and a gyrator.

```
.subckt current_source v1 v2 current
Eg 1 0 {current}
Glcurrent_source 1 0 1
Rlinkage 0 v2 1T
Grcurrent_source v1 v2 1
.ends

Xabc x y current_source param: {i TO current}
```
 - abc is the current source's name
 - x and y are the interconnection nodes
 - i is the electric current crossing the dipole and is constant at all frequencies

#### Ideal Transformer
```
Tlabc x y i
Trabc w z j
```
 - l and r specify which side of the transformer is defined. Both sides must be defined.
 - abc is the transformer's name
 - x and y are the interconnection nodes of the left side of the transformer
 - w and z are the interconnection nodes of the right side of the transformer
 - i and j are the coefficient that represent the windings coupling

#### Gyrator
A gyrator is a theoretical non-reciprocal linear element that reverses the current-voltage characteristic of the circuit between its terminals. It is employed to transition between the impedance and admittance analogies (and vice versa).
```
Glabc x y i
Grabc w z j
```
 - l and r specify which side of the gyrator is defined. Both sides must be defined.
 - abc is the gyrator's name
 - x and y are the interconnection nodes of the left side of the gyrator
 - w and z are the interconnection nodes of the right side of the gyrator
 - i and j are the coefficient that represent the gyration resistance

#### Pistonic radiator
```
Zabc x y surface [ma][ai]
```
 - abc is the radiator's name
 - x and y are the interconnection nodes of the radiator
 - surface is the surface of the circular piston in square meters
 - [ma][ai] are two charaters to select which domain and analogy the dipole conforms to
   * the first character is for the domain selection. 'm' stands for mechanical domain and 'a' stands for acoustic domain
   * the second character is for the analogy selection. 'a' stands for admittance and 'i' stands for impedance

The Piston radiation impedance description may be found in "Electroacoustic modelling of the subwoofer enclosures" chapter 2.5

#### Port (Helmholtz resonator) impedance
```
Pabc x y length surface [kK][kK][dr]
```
 - abc is the port's name
 - x and y are the interconnection nodes of the port
 - surface is the surface of the port in square meters
 - length is the length of the port in meters
 - [kK][kK][dr] are three characters used to select the end correction factors and the exactness of the simulation
   * k stands for free air termination
   * K stands for flanged termination
   * d stands for damped simulation (kind of simplified)
   * r stands for resonant simulation (with resonant harmonics)

The Port inertance impedance may be found in "beranek" chapter 4 eq. 4.1, 4.22 and 4.23

#### Box impedance
```
Babc x y volume
```
 - abc is the box's name
 - x and y are the interconnection nodes of the box
 - volume is the volume of the box in cubic meters. The box impedance is defined as z = 1 / (s* volume/( &rho; * c^2 ) )

#### Semi impedance
```
Kabc x y magnitude pow_of_w [cri][ia]

* example
Ke x y 143m .5 ci
Kams w z 2290 1 ra
```
The semi-impedance dipole is a valuable tool for representing specific dipoles encountered in the frequency-dependent damping modeling of loudspeakers. This particular dipole has been extensively discussed and researched in the paper titled "Frequency Dependence of Damping and Compliance in Loudspeaker Suspensions" by Thorborg, Knud; Tinggaard, Carsten; Agerkvist, Finn; Futtrup, Claus, published in the Journal of the Audio Engineering Society (JAES), Volume 58, Issue 6, pages 472-486 in June 2010.

 - abc is the semi impedance name
 - x and y are the interconnection nodes of the dipole
 - magnitude is the nominal value of the dipole
 - pow_of_w; specifies to which power the angular velocity (&omega;) is elevated
 - [cri] permits to select the form of the number to compute
  - c gives z = magnitude * &omega;^pow_of_w * ( 1 + I )
  - r gives z = magnitude * &omega;^pow_of_w
  - i gives z = magnitude * &omega;^pow_of_w * I
 - [ia] permits to select the analogy of the dipole (e.g. impedance/admitance)
  - i sets the dipole value to z
  - a sets the dipole value to 1 / z

In the examples,
 - Ke defines a semi inductance of .143 SH ( z = 143m * &radic;(&omega;) + I * 143m * &radic;(&omega;) )
 - Kams defines a conductance varying with frequency ( z = 1 / ( 2290 * &omega; ) )

#### Free air impedance
(deprecated)
The inclusion of this one was a mistake

### The Forth-like language
The embedded Forth-like interpreter within yanapack serves various purposes within the circuit file or during interactive sessions (using the `-s` parameter). It is used in the following ways:
 - To display simulation results: The script is embedded in the circuit file, and any lines
   starting with ". " (a dot and a space) are interpreted as Forth code and used to display
   simulation results.
 - To provide parameters to custom sub circuits: Parameters can be specified after the "param:"
   keyword within a block enclosed by curly brackets. This allows customization of sub
   circuits by passing specific values.
 - To push values within instantiated custom sub circuits: The Forth-like interpreter enables
   the insertion of values into the custom sub circuits that are being instantiated. This allows
   for flexible parameterization and customization of the circuits.
 - To display simulation results during an interactive session: In an interactive session, lines
   should not start with a dot. The interpreter will interpret these lines and display the
   corresponding simulation results.

This domain-specific language provides a versatile and flexible approach for analyzing and manipulating simulation results. It serves as a powerful tool that allows for computations and operations to be performed between the simulation and the report, enabling users to extract valuable insights and perform custom calculations.

#### Data types and output

In the software, complex numbers are the sole data type used. The "DOT" word (or its alias ".") is employed to retrieve and display the magnitude of the complex number located at the top of the stack.

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
The comparison is performed on the magnitude of the values in TOS and NOS.
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
 - __IF__: jump to the word following the next ELSE word if the magnitude of the popped value is 0
 - __ELSE__: jump to the word following the next THEN
 - __THEN__: do nothing
 - __BEGIN__: do nothing
 - __WHILE__: jump to the word following the next REPEAT word if popped value is 0
 - __REPEAT__: jump to the last BEGIN word
 - __UNTIL__: jump to the last begin if popped value is 0
 - __AGAIN__: jump to the last begin
 - __LEAVE__: jump to the word following the next REPEAT/UNTIL/AGAIN
 - __BREAK__: LEAVE alias

#### Mathematical and physical constant words
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

In both the Forth context and the netlist definition, only real floating-point numbers are allowed. The numbers are represented in the usual format, and the following examples demonstrate valid figures:
 - 1
 - 1.2
 - .56
 - .68e-3

In addition, there are several suffixes used to perform basic conversions and scaling.
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
