#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <yanapack.h>


#define NETLIST_LOUDSPEAKER			\
  "Eg 0 4 30\n"					\
  "Re 0 1 5.7\n"				\
  "Le 1 2 0.96e-3\n"				\
  "Ra 2 4 120\n"				\
  "La 2 4 96e-3\n"				\
  "Ca 2 4 125e-6\n"				\
  "Cb 2 3 250e-6\n"				\
  "Lb 3 4 300e-6\n"

#define NETLIST_TRANSFORMER			\
  "Eg 1 0 10\n"					\
  "Tl1 1 0 10\n"				\
  "Tr1 2 0 1\n"					\
  "R1 2 0 1\n"					\

#define NETLIST_GYRATOR				\
  "Eg 0 2 5\n"					\
  "Gl1 0 2 7\n"					\
  "Gr1 1 2 7\n"					\
  "R1 1 2 1\n"

#define NETLIST_DIVISOR				\
  "Eg 0 2 10\n"					\
  "R1 0 1 5\n"					\
  "R2 1 2 5\n"

#define NETLIST_BRIDGE				\
  "Eg 0 1 10\n"					\
  "R1 0 2 3\n"					\
  "R2 2 1 7\n"					\
  "R3 2 4 9\n"					\
  "RL 4 1 7\n"					\
  "Rb 0 4 12\n"

#define NETLIST_SIMPLE				\
  "Eg 0 1 10\n"					\
  "R1 0 1 100\n"
#if 0

#define NETLIST_SPLIFFTOWN4000XL		\
  "C1 0 7 22e-6\n"				\
  "C3 0 4 22e-6\n"				\
  "C4 5 6 5.6e-6\n"				\
  "CTmes 0 11 36.7e-6\n"			\
  "CWmes 0 10 495e-6\n"				\
  "Cnotch 6 6 27e-6\n"				\
  "Cx 0 18 1e-6\n"				\
  "L1 2 1 0.8e-3\n"				\
  "L2 0 3 0.35e-3\n"				\
  "LT 9 6 0.006e-3\n"				\
  "LTces 0 11 2.82e-3\n"			\
  "LW 8 7 0.96e-3\n"				\
  "LWces 0 10 96.91e-3\n"			\
  "Lbsw 12 14 1.4e-3\n"				\
  "Lnotch 6 16 .1e-3\n"				\
  "Lx 17 7 2e-3\n"				\
  "R1 4 7 12\n"					\
  "R2 4 7 12\n"					\
  "R3 0 6 18\n"					\
  "RL1 1 7 0.22\n"				\
  "RL2 3 6 0.14\n"				\
  "RLbsw 14 2 0.36\n"				\
  "RLnotch 16 6 .14\n"				\
  "RT 11 9 4.7\n"				\
  "RTes 0 11 29.9\n"				\
  "RW 10 8 5.7\n"				\
  "RWes 0 10 127.5\n"				\
  "Rbst1 15 5 2.35\n"				\
  "Rbst2 12 15 0.9\n"				\
  "Rbsw1 12 13 2.7\n"				\
  "Rbsw2 13 2 2.2\n"				\
  "Rnotch 6 6 5.6\n"				\
  "Rtweeter 0 6 100000e3\n"			\
  "Rwoofer 0 7 100000e3\n"			\
  "Rx 18 17 1\n"				\
  "Vac 12 0 30\n"				\
  "Gl1 0 10 0.035\n"				\
  "Gr1 19 20 1\n"				\
  "RG1 10 20 1e9\n"				\
  "Tl1 19 20 10.1\n"				\
  "Tr1 21 22 1\n"				\
  "RT1 22 20 1e9\n"				\
  "Zrad 21 22 0.035 ai\n"
#else
#define NETLIST_SPLIFFTOWN4000XL		\
						\
  "Eg    1 0   2.83\n"				\
  "Rbsc  1 2   4.9\n"				\
  "Lbsc  1 2   1.4e-3 0.36\n"			\
  "L1    2 3   0.8e-3 0.22\n"			\
  "C1    3 0   22e-6\n"				\
  "R1    3 4   6\n"				\
  "C3    4 0   22e-6\n"				\
  "LeW   3 5   0.96e-3 5.7\n"			\
  "TlW   5 6   10.1\n"				\
						\
  "TrW   6 0   1\n"				\
  "RrmsW 6 0   1.25\n"				\
  "CmmsW 6 0   50.5e-3\n"			\
  "LcmsW 6 0   0.95e-3\n"			\
  "GlW   6 0   0.035\n"				\
						\
  "GrW   7 0   0.035\n"				\
  "Box   7 8   0.037\n"				\
  "ZrW1  8 9  0.035 ai\n"			\
  "ZrW2  9 0 0.035 ai\n"			\
						\
						\
  "RbT   1 11  3.25\n"				\
  "C4    11 12 5.6e-6\n"			\
  "L2    12 0  0.36e-3 0.14\n"			\
  "R3    12 0  18\n"				\
  "ReT   12 13 4.7\n"				\
  "LeT   13 14 0.006e-3\n"			\
  "TlT   14 0  3.5\n"				\
						\
  "TrT   15 0  1\n"				\
  "RrmsT 15 0  2.4390\n"			\
  "CmmsT 15 0  0.45e-3\n"			\
  "LcmsT 15 0  0.23e-3\n"			\
  "GlT   15 0  0.0008\n"			\
						\
  "GrT   16 0  0.0008\n"			\
  "ZrT1  16 17 0.0008 ai\n"			\
  "ZrT2  17 0 0.0008 ai\n"			\
  

#endif

#define NETLIST_SPLIFFTOWN2500XL_IDEAL		\
						\
  "Eg    1 0   2.83\n"				\
  "Rbsc  1 2   4.9\n"				\
  "Lbsc  1 2   1.4e-3 0.36\n"			\
  "L1    2 3   1e-3 0.22\n"			\
  "C1    3 0   12e-6\n"				\
  "R1    3 4   6\n"				\
  "C3    4 0   22e-6\n"				\
  "LeW   3 5   0.96e-3 5.7\n"			\
  "TlW   5 6   10.1\n"				\
						\
  "TrW   6 0   1\n"				\
  "RrmsW 6 0   1.25\n"				\
  "CmmsW 6 0   50.5e-3\n"			\
  "LcmsW 6 0   0.95e-3\n"			\
  "GlW   6 0   0.035\n"				\
						\
  "GrW   7 8   0.035\n"				\
  "ZrW1  7 0  0.035 ai\n"			\
  "ZrW2  8 9 0.035 ai\n"			\
  "Box   9 0   0.025\n"				\
						\
						\
  "RbT   1 10  2.8\n"				\
  "C4    10 11 16.2e-6\n"			\
  "L2    11 0  0.509e-3 0.14\n"			\
  "Rn1   11 12  2.8\n"				\
  "Cn1   12 0   5.102041e-6\n"			\
  "Rn2   11 13 3.594\n"				\
  "Cn2   13 14 170.37e-6\n"			\
  "Ln2   14 0 0.666e-3\n"			\
						\
  "LReT   11 15 0.04e-3 2.8\n"			\
  "TlT   15 0  2.2\n"				\
						\
  "TrT   16 0  1\n"				\
  "RrmsT 16 0  2.041\n"				\
  "CmmsT 16 0  0.42e-3\n"			\
  "LcmsT 16 0  0.27e-3\n"			\
  "GlT   16 0  0.0008\n"			\
						\
  "GrT   17 18  0.0008\n"			\
  "ZrT1  17 0 0.0008 ai\n"			\
  "ZrT2  18 0 0.0008 ai\n"			\

#define NETLIST_SPLIFFTOWN2500XL		\
						\
  "Eg    1 0   2.83\n"				\
  "Rbsc  1 2   4.7\n"				\
  "Lbsc  1 2   1.4e-3 0.42\n"			\
  "L1    2 3   1e-3 0.26\n"			\
  "C1    3 0   12e-6\n"				\
  "R1    3 4   6\n"				\
  "C3    4 0   22e-6\n"				\
  "LeW   3 5   0.96e-3 5.7\n"			\
  "TlW   5 6   10.1\n"				\
						\
  "TrW   6 0   1\n"				\
  "RrmsW 6 0   1.25\n"				\
  "CmmsW 6 0   50.5e-3\n"			\
  "LcmsW 6 0   0.95e-3\n"			\
  "GlW   6 0   0.035\n"				\
						\
  "GrW   7 8   0.035\n"				\
  "ZrW1  7 0  0.035 ai\n"			\
  "ZrW2  8 9 0.035 ai\n"			\
  "Box   9 0   0.025\n"				\
						\
						\
  "RbT   1 10  2.7\n"				\
  "C4    10 11 15e-6\n"				\
  "L2    11 0  0.5e-3 0.21\n"			\
  "Rn1   11 12  3.3\n"				\
  "Cn1   12 0   5.6e-6\n"			\
  "Rn2   11 13 3.9\n"				\
  "Cn2   13 14 172e-6\n"			\
  "Ln2   14 0 0.600e-3\n"			\
						\
  "LReT   11 15 0.04e-3 2.8\n"			\
  "TlT   15 0  2.2\n"				\
						\
  "TrT   16 0  1\n"				\
  "RrmsT 16 0  2.041\n"				\
  "CmmsT 16 0  0.42e-3\n"			\
  "LcmsT 16 0  0.27e-3\n"			\
  "GlT   16 0  0.0008\n"			\
						\
  "GrT   17 18  0.0008\n"			\
  "ZrT1  17 0 0.0008 ai\n"			\
  "ZrT2  18 0 0.0008 ai\n"			\

#define NETLIST_PORTED_XLS12_1			\
  "Eg 1 0 2.83\n"				\
  "Re 1 2 3.6\n"				\
  "Le 2 3 .70e-3\n"                             \
  "Tl1 3 0 18.53\n"				\
  "Tr1 4 5 1\n"					\
  "RT1 0 5 1e9\n"				\
  "Rrms 4 5 .1795245\n"				\
  "Cmms 4 5 193.9e-3\n"				\
  "Lcms 4 5 317e-6\n"				\
  "Gl1 4 5 0.04831\n"				\
  "Gr1 6 7 1\n"					\
  "RG1 5 7 1e9\n"				\
  "Zrad 6 10 0.0481 ai\n"			\
  "Rab 10 8 441.0937\n"				\
  "Ccab 8 7 7.0552e-7\n"			\
  "Ral 10 7 90234.1\n"				\
  "Rap 10 9 90.234\n"				\
  "Lmap 9 7 57.4447\n"

#define NETLIST_PORTED_XLS12_2			\
  "Eg     1 0  2.83\n"				\
  "Re     1 2  3.6\n"				\
  "Le     2 3  0.7e-3\n"			\
  "Tl1    3 0  18.53\n"				\
  						\
  "Tr1    4 0  1\n"				\
  "Rrms   4 0  0.179524488\n"			\
  "Cmms   4 0  193.9e-3\n"			\
  "Lcms   4 0  .317e-3\n"			\
  "Gl1    4 0  0.04831\n"			\
  						\
  "Gr1    5 6  0.04831\n"			\
  "Zradf  5 0  0.04831 ai\n"			\
  "Zradr  6 7  0.04831 ai\n"			\
  "Box    7 8  0.100\n"				\
  "Rab    8 0 451.17\n"				\
  "Ral   7 0 90234.1\n"				\
  "Port  7 0 0.2368 78.54e-4 kK\n"		\
  /*
  "Zport 9 0  78.54e-4 ai\n"			\
  */
#define NETLIST_XLS12_FA			\
  "Eg     1 0  2.83\n"				\
  "Re     1 2  3.5\n"				\
  "Le     2 3  4.2e-3\n"			\
  "Tl1    3 0  17.6\n"				\
  						\
  "Tr1    4 0  1\n"				\
  "Rrms   4 0  0.1953125\n"			\
  "Cmms   4 0  166.3e-3\n"			\
  "Lcms   4 0  .46e-3\n"			\
  "Gl1    4 0  0.0466\n"			\
  						\
  "Gr1    5 0  0.0466\n"			\
  "Zradf  5 6  0.0466 ai\n"			\
  "Zradr  6 7  0.0466 ai\n"			\
  "Adiaph 7 0  1 2\n"				\

#define NETLIST_LAB12_FA			\
  "Eg     1 0  2.83\n"				\
  "Re     1 2  4.29\n"				\
  "Le     2 3  1.48e-3\n"			\
  "Tl1    3 0  15\n"				\
  						\
  "Tr1    4 0  1\n"				\
  "Rrms   4 0  .66\n"				\
  "Cmms   4 0  146e-3\n"			\
  "Lcms   4 0  .35e-3\n"			\
  "Gl1    4 0  0.05067\n"			\
  						\
  "Gr1    5 0  0.05067\n"			\
  "Zradf  5 6  0.05067 ai\n"			\
  "Zradr  6 7  0.05067 ai\n"			\
  "Adiaph 7 8  1 2\n"				\
  "Box    8 0  0.1\n"				\
  "Port   8 9 0.25 78.54e-4 kK\n"		\
  "Zport  9 10  78.54e-4 ai\n"			\
  "Aport 10 0  1 2\n"				\


  /*
  "Box 8 0 100.68e-3\n"				\
						\
						\
  */
/*
  "Zport 10 11 78.54e-4 ai\n"			\
  "Aport 11 0 1 2\n"
*/
#define NETLIST_ZRAD				\
  "Eg 0 1 1\n"					\
  "Zrad 0 1 0.035 ai\n"
#if 1
#define NETLIST_26W8534G00		\
  "Eg    1 0  2.82843\n"			\
  "Re    1 2  5.7\n"			\
  "Le    2 3  .96e-3\n"			\
  "Tl1   3 0  10.1\n"			\
  "Tr1   4 0  1\n"				\
  "Rrms  4 0  1.25\n"			\
  "Cmms  4 0  50.5e-3\n"			\
  "Lcms  4 0  0.95e-3\n"			\
  "Gl1   4 0  0.035\n"			\
  "Gr1   5 0  0.035\n"			\
  "Box   5 6  0.037\n"			\
  "Zrad  6 7  0.035 ai\n"	/* front */	\
  "Zrad  7 8  0.035 ai\n" /* rear */ 	\
  "Alist 8 0  1 2\n"
#endif

#if 0
#define NETLIST_26W8534G00		\
  "Eg 1 0 2.83\n"			\
  "Re 1 2 5.7\n"			\
  "Le 2 3 .96e-3\n"			\
  "Gl1 3 0 3.465347e-3\n"		\
  "Gr1 5 0 3.465347e-3\n"		\
  "Rrms 5 6 653.06122\n"		\
  "Lmms 6 7 41.2244898\n"		\
  "Ccms 7 8 1.16375e-6\n"		\
  "Box 8 9 0.037\n"			\
  "Zrad1 9 10 0.035 ai\n"		\
  "Zrad2 10 11 0.035 ai\n"		\
  "Alist 11 0 1 2\n"
#endif

#define NETLIST_D2604_8330			\
  "Eg    1 0 2.83\n"				\
  						\
  "Rn1   1 2  2.8\n"				\
  "Cn1 2 0 5.102041e-6\n"			\
  						\
  "Rn2   1 3 3.594\n"				\
  "Cn2   3 4 170.37e-6\n"			\
  "Ln2   4 0 0.666e-3\n"			\
  						\
  "LReT  1 5 0.04e-3 2.8\n"			\
  "TlT   5 0  2.2\n"				\
  						\
  "TrT   6 0  1\n"				\
  "RrmsT 6 0  2.041\n"				\
  "CmmsT 6 0  0.42e-3\n"			\
  "LcmsT 6 0  0.27e-3\n"			\
  "GlT   6 0  0.0008\n"				\
  						\
  "GrT   7 8  0.0008\n"				\
  "ZrT1  7 0 0.0008 ai\n"			\
  "ZrT1  8 0 0.0008 ai\n"





#define NETLIST_TWIN_GYRATOR			\
  "Eg  1 0 4\n"					\
  "L1  1 2 0.8e-3\n"				\
  "Te1 2 0 1\n"					\
  "Ts1 3 0 1\n"					\
  "Ge1 3 0 1\n"					\
  "Gs1 4 0 1\n"					\
  "L2  4 5 22e-6\n"				\
  "R1  5 0 .17543859649122807017\n"		\
  						\
						\
  "C1  1 6 22e-6\n"				\
  "Te2 6 0 1\n"					\
  "Ts2 7 0 1\n"					\
  "Ge2 7 0 1\n"					\
  "Gs2 8 0 1\n"					\
  "C2  8 9 0.8e-3\n"				\
  "R2  9 0 .17543859649122807017\n"

#define ASSERT_EQUALS(X,Y)						\
  do { yana_complex_t x = X;						\
    yana_complex_t y = Y;						\
    if ( !( fabs(creal(x)-creal(y)) < 0.0001				\
	    && fabs(cimag(x)-cimag(y)) < 0.0001 ) )			\
      {									\
	fprintf(stderr, "v1=%.18f+i%.18f != v2=%.18f+i%.18f\n",		\
		creal(x),cimag(x),creal(y),cimag(y));			\
	assert(! #X " != " #Y );					\
      }									\
  }									\
  while (0)


void utest_transformer(void)
{
  simulation_context_t *sc = simulation_context_new(1,1,0);
  netlist_t *netlist;
  nodelist_t *nodelist;
  simulation_t *simulation;
  status_t status;
  status = netlist_new(sc, NETLIST_TRANSFORMER, &netlist);
  assert( SUCCESS == status );
  status = nodelist_new(sc, netlist, &nodelist);
  simulation_new(sc, nodelist, &simulation);
  simulation_set_values(simulation);
  simulation_solve(simulation);

  //ve
  ASSERT_EQUALS(simulation->cells[1][simulation->n_vars].values[0]
		-simulation->cells[0][simulation->n_vars].values[0],
		10.L + I * 0.L);
  //vs
  ASSERT_EQUALS(simulation->cells[2][simulation->n_vars].values[0]
		-simulation->cells[0][simulation->n_vars].values[0],
		1L + I * 0.L);
  //Ieg
  ASSERT_EQUALS(simulation->cells[3][simulation->n_vars].values[0],-0.1L + I * 0);
  //ITl1
  ASSERT_EQUALS(simulation->cells[4][simulation->n_vars].values[0],0.1L + I * 0);
  //ITr1
  ASSERT_EQUALS(simulation->cells[5][simulation->n_vars].values[0], -1.L + I * 0);
  //IR1
  ASSERT_EQUALS(simulation->cells[6][simulation->n_vars].values[0], 1.L + I * 0);
}

void utest_gyrator(void)
{
  simulation_context_t *sc = simulation_context_new(1,1,0);
  netlist_t *netlist;
  nodelist_t *nodelist;
  simulation_t *simulation;
  status_t status;
  status = netlist_new(sc, NETLIST_GYRATOR, &netlist);
  assert( SUCCESS == status );
  status = nodelist_new(sc, netlist, &nodelist);
  simulation_new(sc, nodelist, &simulation);
  simulation_set_values(simulation);
  simulation_solve(simulation);

  //ve
  ASSERT_EQUALS(simulation->cells[0][simulation->n_vars].values[0]
		-simulation->cells[2][simulation->n_vars].values[0],
		5.L + I * 0.L);
  //IR1
  ASSERT_EQUALS(simulation->cells[6][simulation->n_vars].values[0],
		5.L * 7.L + I * 0.L);
  //IGr1
  ASSERT_EQUALS(simulation->cells[5][simulation->n_vars].values[0],
		-35.L + I * 0.L);

  //vs *** R=1
  ASSERT_EQUALS(simulation->cells[6][simulation->n_vars].values[0],
		35.L + I * 0.L);
  //IGl1
  ASSERT_EQUALS(simulation->cells[4][simulation->n_vars].values[0],
		(245.L + I * 0.L));

  //IEg
  ASSERT_EQUALS(simulation->cells[3][simulation->n_vars].values[0],
		-(245.L + I * 0.L));
  
}

void utest_par_on_eg(void)
{
  simulation_context_t *sc = simulation_context_new(1,1,0);
  netlist_t *netlist;
  nodelist_t *nodelist;
  simulation_t *simulation;
  status_t status;
  status = netlist_new(sc,
		       "Eg 1 0 10\n"
		       "R1 1 2 5\n"
		       "R2 2 0 5\n"
		       "R3 1 3 5\n"
		       "R4 3 0 5\n"
		       , &netlist);
  assert( SUCCESS == status );
  status = nodelist_new(sc, netlist, &nodelist);
  simulation_new(sc, nodelist, &simulation);
  simulation_set_values(simulation);
  simulation_solve(simulation);

  //v12
  ASSERT_EQUALS(simulation->cells[1][simulation->n_vars].values[0]
		-simulation->cells[2][simulation->n_vars].values[0],
		5.L + I * 0.L);
  //v20
  ASSERT_EQUALS(simulation->cells[2][simulation->n_vars].values[0]
		-simulation->cells[0][simulation->n_vars].values[0],
		5.L + I * 0.L);
  //v13
  ASSERT_EQUALS(simulation->cells[1][simulation->n_vars].values[0]
		-simulation->cells[3][simulation->n_vars].values[0],
		5.L + I * 0.L);
  //v30
  ASSERT_EQUALS(simulation->cells[3][simulation->n_vars].values[0]
		-simulation->cells[0][simulation->n_vars].values[0],
		5.L + I * 0.L);

  //IR1
  ASSERT_EQUALS(simulation->cells[5][simulation->n_vars].values[0],
		1.L + I * 0.L);

  //IR2
  ASSERT_EQUALS(simulation->cells[6][simulation->n_vars].values[0],
		1.L + I * 0.L);
  //IR3
  ASSERT_EQUALS(simulation->cells[7][simulation->n_vars].values[0],
		1.L + I * 0.L);
  //IR4
  ASSERT_EQUALS(simulation->cells[8][simulation->n_vars].values[0],
		1.L + I * 0.L);
}

void utest_div_network(void)
{
  simulation_context_t *sc = simulation_context_new(1,1,0);
  netlist_t *netlist;
  nodelist_t *nodelist;
  simulation_t *simulation;
  status_t status;
  status = netlist_new(sc,
		       "Eg 1 0 256\n"
		       "L1 1 2 1\n"
		       "L1b 2 0 2\n"
		       "L2 2 3 1\n"
		       "L2b 3 0 2\n"
		       "L3 3 4 1\n"
		       "L3b 4 0 2\n"
		       "L4 4 5 1\n"
		       "L4b 5 0 2\n"
		       "L5 5 6 1\n"
		       "L5b 6 0 2\n"
		       "L6 6 7 1\n"
		       "L6b 7 0 2\n"
		       "L7 7 8 1\n"
		       "L7b 8 0 2\n"
		       "L8 8 9 1\n"
		       "L8b 9 0 1\n"

		       "CA 1 10 2000\n"
		       "CAb 10 0 1000\n"
		       "CB 10 11 2000\n"
		       "CBb 11 0 1000\n"
		       "CC 11 12 2000\n"
		       "CCb 12 0 1000\n"
		       "CD 12 13 2000\n"
		       "CDb 13 0 1000\n"
		       "CE 13 14 2000\n"
		       "CEb 14 0 1000\n"
		       "CF 14 15 2000\n"
		       "CFb 15 0 1000\n"
		       "CG 15 16 2000\n"
		       "CGb 16 0 1000\n"
		       "CH 16 17 2000\n"
		       "CHb 17 0 2000\n"

		       , &netlist);
  assert( SUCCESS == status );
  status = nodelist_new(sc, netlist, &nodelist);
  simulation_new(sc, nodelist, &simulation);
  simulation_set_values(simulation);

  
  simulation_solve(simulation);

  //vfinal
  ASSERT_EQUALS(simulation->cells[9][simulation->n_vars].values[0]
		-simulation->cells[0][simulation->n_vars].values[0],
		1.L + I * 0.L);
  //vfinal
  ASSERT_EQUALS(simulation->cells[17][simulation->n_vars].values[0]
		-simulation->cells[0][simulation->n_vars].values[0],
		1.L + I * 0.L);

}


void utest_divisor(void)
{
  simulation_context_t *sc = simulation_context_new(1,1,0);
  netlist_t *netlist;
  nodelist_t *nodelist;
  simulation_t *simulation;
  status_t status;
  status = netlist_new(sc, NETLIST_DIVISOR, &netlist);
  assert( SUCCESS == status );
  status = nodelist_new(sc, netlist, &nodelist);
  simulation_new(sc, nodelist, &simulation);
  simulation_set_values(simulation);
  simulation_solve(simulation);

  //ve
  ASSERT_EQUALS(simulation->cells[0][simulation->n_vars].values[0]
		-simulation->cells[2][simulation->n_vars].values[0],
		10.L + I * 0.L);
  //V01
  ASSERT_EQUALS(simulation->cells[0][simulation->n_vars].values[0]
		-simulation->cells[1][simulation->n_vars].values[0],
		5.L + I * 0.L);

  //V12
  ASSERT_EQUALS(simulation->cells[1][simulation->n_vars].values[0]
		-simulation->cells[2][simulation->n_vars].values[0],
		5.L + I * 0.L);
  //Ir1
  ASSERT_EQUALS(simulation->cells[4][simulation->n_vars].values[0],
		1L + I * 0.L);

  //Ir1
  ASSERT_EQUALS(simulation->cells[5][simulation->n_vars].values[0],
		1L + I * 0.L);
  //Ieg
  ASSERT_EQUALS(simulation->cells[3][simulation->n_vars].values[0],
		-1.L + I * 0.L);

}

int main(int argc, char **argv)
{
  simulation_context_t *sc = simulation_context_new(1,5,500);
  //simulation_context_t *sc = simulation_context_new(-2,2,100);
  netlist_t *netlist;
  nodelist_t *nodelist;
  simulation_t *simulation;
  status_t status; 
  int dump=0;
  
#if 0
  utest_transformer();
  utest_gyrator();
  utest_divisor();
  utest_par_on_eg();
  utest_div_network();
  exit(0);
#endif

  status = netlist_new(sc,
		       //NETLIST_SIMPLE
		       //NETLIST_BRIDGE
		       //NETLIST_DIVISOR
		       //NETLIST_LOUDSPEAKER
		       //NETLIST_SPLIFFTOWN4000XL
		       NETLIST_SPLIFFTOWN2500XL
		       //NETLIST_SPLIFFTOWN2500XL_IDEAL
		       //NETLIST_D2604_8330
		       //NETLIST_26W8534G00
		       //NETLIST_TRANSFORMER
		       //NETLIST_GYRATOR
		       //NETLIST_ZRAD
		       //NETLIST_PORTED_XLS12_2
		       //NETLIST_XLS12_FA
		       //NETLIST_LAB12_FA
		       //NETLIST_TWIN_GYRATOR
		       ,
		       &netlist);
  assert(status == SUCCESS );
  status = nodelist_new(sc, netlist, &nodelist);
  simulation_new(sc, nodelist, &simulation);

  if (dump)
    {
      netlist_dump(netlist);
      nodelist_dump(nodelist);
    }
  simulation_set_values(simulation);
  if (dump) simulation_dump(simulation);

  simulation_solve(simulation);
  if (dump) simulation_dump(simulation);

  //if (dump) simulation_dump_names(simulation);

  if (!dump)
    {
      int s = simulation_context_get_n_samples(sc);
      int i;
      for ( i = 0 ; i < s ; ++i )
	{
#if 0
	  //puissance
	  yana_complex_t diaph =
	    (simulation->cells[6][simulation->n_vars].values[i] - simulation->cells[10][simulation->n_vars].values[i] );
	    /* * -simulation->cells[27][simulation->n_vars].values[i] */;
	  yana_complex_t port= (simulation->cells[9][simulation->n_vars].values[i] - simulation->cells[7][simulation->n_vars].values[i] );
	  yana_complex_t sum = diaph+port;
	  yana_real_t mod_diaph = sqrt( pow(creal(diaph),2.) + pow(cimag(diaph),2.));
	  yana_real_t mod_port = sqrt( pow(creal(port),2.) + pow(cimag(port),2.));
	  yana_real_t mod_sum = sqrt( pow(creal(sum),2.) + pow(cimag(sum),2.));

	  printf("%f\t%f\t%f\t%f\n",simulation_context_get_f(sc,i),
		 20. * log10( mod_diaph /(2.L*M_PI) ),
		 20. * log10( mod_port/(2.L*M_PI) ),
		 20. * log10( mod_sum/(2.L*M_PI) )
		 );
#endif


#if 0
	  //laplace
	  yana_real_t *result;
	  int steps;
	  inv_laplace(sc, simulation_result(simulation, "Eg"), &result, &steps);
	  for ( i = 0 ; i < steps ; ++i )
	    {
	      printf("%d\t%f\n",i,(double)result[i]);
	    }
	  break;

#endif



#if 0
	  //impedance
	  yana_complex_t imp =
	    (simulation_result(simulation, "1")[i] - simulation_result(simulation, "0")[i])
	    / ( simulation_result(simulation, "Eg")[i] );
	  yana_real_t mod = sqrt( pow(creal(imp),2.) + pow(cimag(imp),2.));

	  printf("%f\t%f\t%f\t%f\n",(double)simulation_context_get_f(sc,i),
		 (double)mod,
		 (double)fabs(creal(imp)),
		 (double)fabs(cimag(imp)));

#endif

#if 0
	  //autre
	  yana_complex_t v1 = 
	    (simulation->cells[17][simulation->n_vars].values[i]);// - simulation->cells[0][simulation->n_vars].values[i] );
	  yana_complex_t v2 =
	    (simulation->cells[24][simulation->n_vars].values[i]);
	  yana_real_t mod1 = sqrt( pow(creal(v1),2.) + pow(cimag(v1),2.));
	  yana_real_t mod2 = sqrt( pow(creal(v2),2.) + pow(cimag(v2),2.));

	  printf("%f\t%f\t%fpi\t%f\t%fpi\n",
		 (double)simulation_context_get_f(sc,i),
		 (double)mod1,carg(v1) / M_PI,
		 (double)mod2,carg(v2) / M_PI);


#endif

#if 0
	  //TRANSFORMER
	  /*
	  printf("v0:\t%f\n", creal(simulation->cells[0][simulation->n_vars].values[i]));
	  printf("v1:\t%f\n", creal(simulation->cells[1][simulation->n_vars].values[i]));
	  printf("v2:\t%f\n", creal(simulation->cells[2][simulation->n_vars].values[i]));
	  */
	  printf("ve:\t%f\n", creal(simulation->cells[1][simulation->n_vars].values[i]-simulation->cells[0][simulation->n_vars].values[i]));
	  printf("vs:\t%f\n", creal(simulation->cells[2][simulation->n_vars].values[i]-simulation->cells[0][simulation->n_vars].values[i]));
	  printf("IEg:\t%f\n", creal(simulation->cells[3][simulation->n_vars].values[i]));
	  printf("ITl1:\t%f\n", creal(simulation->cells[4][simulation->n_vars].values[i]));
	  printf("ITr1:\t%f\n", creal(simulation->cells[5][simulation->n_vars].values[i]));
	  printf("IR1:\t%f\n", creal(simulation->cells[6][simulation->n_vars].values[i]));
	  break;
#endif

#if 0
	  // SPIFFTOWN4000XL
	  yana_complex_t imp = free_air_impedance(simulation_context_get_f(sc,i), 1, 2);

	  //radiation
	  yana_complex_t i1 = simulation_result(simulation,"ZrW1")[i];
	  yana_complex_t i2 = simulation_result(simulation,"ZrT1")[i];
	  yana_complex_t v1 = imp * i1;
	  yana_complex_t v2 = imp * i2;

	  yana_complex_t sum = v1 + v2;
	  yana_real_t mod1 = cabs(v1);
	  yana_real_t mod2 = cabs(v2);
	  yana_real_t mods = cabs(sum);

# if 1
	  printf("%f\t%.18f\t%.18f\t%.18f\n",
		 (double)simulation_context_get_f(sc,i),
		 (double)20. * log10( mods  / (  20e-6)   ),
		 (double)20. * log10( mod1  / (  20e-6)  ),
		 (double)20. * log10( mod2 / (  20e-6)  )
		 );
# else
	  printf("%f\t%.18f\t%.18f\t%.18f\n",
		 (double)simulation_context_get_f(sc,i),
		 (double)(coef * mod),
		 (double)fabs(creal(v)),
		 (double)fabs(cimag(v))
		 );

# endif
#endif


#if 1
	  // SPIFFTOWN2500XL
	  yana_complex_t imp = free_air_impedance(simulation_context_get_f(sc,i), 1, 2);
	  //radiation
	  yana_complex_t i1 = simulation_result(simulation,"ZrW1")[i];
	  yana_complex_t i2 = simulation_result(simulation,"ZrT1")[i];
	  yana_complex_t v1 = imp * i1;
	  yana_complex_t v2 = imp * i2;

	  yana_complex_t sum = v1 - v2;
	  yana_real_t mod1 = cabs(v1);
	  yana_real_t mod2 = cabs(v2);
	  yana_real_t mods = cabs(sum);

# if 1
	  printf("%f\t%.18f\t%.18f\t%.18f\n",
		 (double)simulation_context_get_f(sc,i),
		 (double)20. * log10( mods  / (  20e-6)   ),
		 (double)20. * log10( mod1  / (  20e-6)  ),
		 (double)20. * log10( mod2 / (  20e-6)  )
		 );
# else
	  printf("%f\t%.18f\t%.18f\t%.18f\n",
		 (double)simulation_context_get_f(sc,i),
		 (double)(coef * mod),
		 (double)fabs(creal(v)),
		 (double)fabs(cimag(v))
		 );

# endif
#endif


#if 0
	  //D2604
	  yana_complex_t imp = free_air_impedance(simulation_context_get_f(sc,i),
						1, 2);
	  yana_complex_t i1 =
	    simulation->cells[16][simulation->n_vars].values[i];
	  yana_complex_t v1 = imp * i1;
	  printf("%f\t%.18f\t%.18f\t%.18f\n",
		 (double)simulation_context_get_f(sc,i),
		 (double)20. * log10( cabs(v1)  / (  20e-6)   ),
		 (double)20. * log10( creal(v1)  / (  20e-6)  ),
		 (double)20. * log10( cimag(v1) / (  20e-6)  )
		 );

#endif

#if 0
	  //XLS12 2
	  yana_complex_t imp = free_air_impedance(simulation_context_get_f(sc,i),
						1, 2);

	  yana_complex_t i1 =
	    simulation->cells[19][simulation->n_vars].values[i];
	  yana_complex_t i2 =
	    simulation->cells[24][simulation->n_vars].values[i];
	  
	  yana_complex_t sum = imp *( i1 + i2 );
	  yana_real_t mv1 = cabs(sum);

	  printf("%f\t%.18f\t%.18f\t%.18f\n",
		 (double)simulation_context_get_f(sc,i),
		 (double)20. * log10( mv1  / (  20e-6)   ),
		 (double)20. * log10( cabs(imp*i1)  / (  20e-6)  ),
		 (double)20. * log10( cabs(imp*i2) / (  20e-6)  )
		 );
	  
#endif



	}
    }
  simulation_free(simulation);
  simulation_context_free(sc);
  return status;
}
