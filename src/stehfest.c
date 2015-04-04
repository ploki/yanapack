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
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include "yanapack.h"

static inline double fact(double x)
{
  return tgamma(x+1.L);
}

static double yy1(double k, double N, double i)
{
  return pow(k, N/2.L)*fact(2.L*k)/
    (fact(N/2.L-k)*fact(k)*fact(k-1.L)*fact(i-k)*fact(2.L*k-i));
}

static double v(int i, int N)
{
  double sigma = 0;
  int k_min = floor((double)(i+1)/2.L);
  int k_max;
  int k;
  if ( i > N/2 )
    k_max = N/2;
  else
    k_max = i;

  for ( k = k_min ; k <= k_max ; ++k )
    sigma += yy1(k, N, i);
  
  return pow(-1.L, N/2.L+i)*sigma;
}

__attribute__((unused))
static void utest_v1()
{
  int i;
  double expected_results[] = { 0, //unused
			      1./12.,
			      -385./12.,
			      1279.,
			      -46871./3.,
			      505465./6.,
			      -473915/2.,
			      1127735./3.,
			      -1020215./3.,
			      328125./2.,
			      -65625./2. };
  for ( i = 1 ; i < 10 ; ++i )
    {
      double result = v(i,10);
      assert( fabs(result-expected_results[i]) < 0.0001 );
    }
  assert( fabs(v(18,20)-2.333166532137059*pow(10.,11)) < 0.001 );
}

/*
  f = M_LN2 / t
  t = M_LN2 / f
  
  u( M_LN2 / f ) = f * sigma [j=1..N] ( Vj * U(j*f) )
 */

void inv_laplace(simulation_context_t *sc, yana_complex_t *values,
		 yana_real_t **resultp, int *stepp)
{
  utest_v1();
  //__asm__("int3");
  assert( NULL != resultp );
  assert( NULL != stepp );
  yana_real_t step = 1.L/simulation_context_get_f(sc,
						  simulation_context_get_n_samples(sc)-1);
  yana_real_t length = 1.L/simulation_context_get_f(sc,0);
  int steps = length/step;
  int i, j;
  static const int N = 20;
  yana_real_t *result = malloc(sizeof(yana_real_t)*steps);
  for ( i = 0 ; i < steps ; ++i )
    {
      int sample;
      yana_real_t f, harmonic;
      yana_real_t sigma = 0;
      f = M_LN2/((i+1)*step);
      for ( j = 1 ; j <= N ; ++j )
	{
	  harmonic = j * f / 2.L / M_PI;
	  sample = simulation_context_get_sample(sc, harmonic);
	  if ( sample < 0 ) break;
	  sigma += v(j, N) * cabs(values[sample]);
	}
      result[i] = f * sigma;
    }
  *resultp = result;
  *stepp = steps;
}

/*
int main(int argc, char **argv)
{
  utest_v1();
  printf("v%d = %f\n",1,v(3,10));
}
*/
