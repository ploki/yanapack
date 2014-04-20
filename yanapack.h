#ifndef __YANAPACK_H__
#define __YANAPACK_H__ 1

#include <math.h>
#include <complex.h>

typedef enum
  {
    SUCCESS,
    FAILURE
  } status_t;



#if 0
#define YANA_RHO (1.20L)
#define YANA_C   (343.L)
#else
#define YANA_RHO (1.184L)
#define YANA_C   (346.1L)
#endif

// N.s.m^-2, viscosity coefficient
#define YANA_MU         (1.86e-5L)

#define YANA_FREE_AIR_K (0.307L)
#define YANA_FLANGED_K  (0.425L)


#if 1
typedef long double complex yana_complex_t;
typedef long double yana_real_t;
#else
typedef double complex yana_complex_t;
typedef double yana_real_t;
#endif


#include "yana_map.h"
#include "simulation_context.h"
#include "dipole.h"
#include "netlist.h"
#include "nodelist.h"
#include "simulation.h"
#include "solver.h"
#include "stehfest.h"

#endif
