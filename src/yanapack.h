/* 
 * Copyright (c) 2013-2014, Guillaume Gimenez <guillaume@blackmilk.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of G.Gimenez nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL G.GIMENEZ BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     * Guillaume Gimenez <guillaume@blackmilk.fr>
 *
 */
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

#define YANA_RHO_C (YANA_RHO*YANA_C)

// N.s.m^-2, viscosity coefficient
#define YANA_MU         (1.86e-5L)

#if 1
// beranek p. 121 eq. 4.5 and 4.8
# define YANA_FREE_AIR_K (0.640L)
# define YANA_FLANGED_K  (0.850L)
#else
// don't remember the source
# define YANA_FREE_AIR_K (0.307L)
# define YANA_FLANGED_K  (0.425L)
#endif


#if 0
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
