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
#include <yanapack.h>
#include <stdio.h>
#include <assert.h>

#define DISABLE_ISFINITE 1

#define USE_OMP

#ifdef DISABLE_ISFINITE
# ifdef isfinite
#  undef isfinite
# endif
# define isfinite(x) (1)
#endif

#if 0
static inline yana_complex_t c_sub(yana_complex_t l, yana_complex_t r)
{
  assert( isfinite(creal(l)) );
  assert( isfinite(cimag(l)) );
  assert( isfinite(creal(r)) );
  assert( isfinite(cimag(r)) );
  return l - r;
}

static inline yana_complex_t c_div(yana_complex_t l, yana_complex_t r)
{
  assert( isfinite(creal(l)) );
  assert( isfinite(cimag(l)) );
  assert( isfinite(creal(r)) );
  assert( isfinite(cimag(r)) );
  assert( fabs(creal(r)) > 1e-18L || fabs(cimag(r)) > 1e-18L );
  return l / r;
}

static inline yana_complex_t c_mul(yana_complex_t l, yana_complex_t r)
{
  assert( isfinite(creal(l)) );
  assert( isfinite(cimag(l)) );
  assert( isfinite(creal(r)) );
  assert( isfinite(cimag(r)) );
  return l * r;
}

static inline yana_complex_t c_let(yana_complex_t l)
{
  assert( isfinite(creal(l)) );
  assert( isfinite(cimag(l)) );
  return l;
}

#else
#define c_sub(l,r) ((l)-(r))
#define c_div(l,r) ((l)/(r))
#define c_mul(l,r) ((l)*(r))
#define c_let(l) (l)
#endif


int cell_cmp(simulation_t *simulation, cell_t *lhs, cell_t *rhs)
{
  int s = simulation_context_get_n_samples(simulation->nodelist->netlist->sc);
  int i;
  yana_complex_t sl = 0;
  yana_complex_t sr = 0;
  yana_real_t ml, mr;
  for ( i = 0 ; i < s ; ++i )
    {
      sl += lhs->values[i];
      sr += rhs->values[i];
    }
  sl/=fabs((yana_real_t)s);
  sr/=fabs((yana_real_t)s);

  ml = sqrt(pow(creal(sl),2.L)+pow(cimag(sl),2.L));
  mr = sqrt(pow(creal(sr),2.L)+pow(cimag(sr),2.L));

  if ( ml > mr )
    return 1;
  else if ( ml < mr )
    return -1;
  else
    return 0;
}


int simulation_max(simulation_t *simulation, int k, int m)
{
  int i;
  int i_max;
  yana_real_t v_max;
  yana_real_t tmp;

  i_max = k;
  v_max = cell_mean_module(simulation, &simulation->cells[i_max][k]);
  for ( i = k ; i < m ; ++i )
    {
      tmp = cell_mean_module(simulation, &simulation->cells[i][k]);
      if ( tmp > v_max )
	{
	  v_max = tmp;
	  i_max = i;
	}
    }
  //printf("col %d, (from row %d to %d) max is row %d with %.1f\n",k, k, m-1, i_max, v_max);
  return i_max;
}


void simulation_solve(simulation_t *simulation)
{
  int m,s;
  int i,j,k, i_max, x;
  m = simulation->n_vars;
  int n = m+1;
  s = simulation_context_get_n_samples(simulation->nodelist->netlist->sc);

  for ( k = 0 ; k < m ; ++k )
    {
      //pivot is col k
      //find a line with the maximum k
      i_max = simulation_max(simulation, k,m);

      if ( cell_is_zero(simulation, &simulation->cells[i_max][k]) )
	{
	  fprintf(stderr,"singular matrix, zero found @ row %d col %d (mean = %.1f)\n", i_max, k,
		  (double)cell_mean(simulation,&simulation->cells[i_max][k]));
	  simulation_dump(simulation);
	  assert(0);
	}
      assert(i_max >= k);

      if ( i_max != k )
	{
	  cell_t *tmp;
	  tmp = simulation->cells[k];
	  simulation->cells[k] = simulation->cells[i_max];
	  simulation->cells[i_max] = tmp;
	}


      for ( i = k + 1 ; i < m ; i ++ )
	{
	  if ( simulation->cells[i][k].state == CELL_ZERO )
	    {
	      //fprintf(stderr,"continue\n");
	      continue;
	    }
	  assert( i != k);
#ifdef USE_OMP
#pragma omp parallel for
#endif
	  for ( j = n - 1 ;  j > k  ; --j )
	    {
	      // Fij = Fij - Fkj * Fik/Fkk
	      assert( j != k );
	      cell_t *line_i = simulation->cells[i];
	      cell_t *line_k = simulation->cells[k];
	      line_i[j].state = CELL_SET;
	      for ( x = 0 ; x < s ; ++x )
		{
		  //assert( line_k[k].state != CELL_ZERO && !cell_is_zero(simulation,&line_k[k]));
		  line_i[j].values[x] = c_let(
		    c_sub(line_i[j].values[x],
			  c_mul(line_k[j].values[x],
				c_div( line_i[k].values[x], line_k[k].values[x] ))));
		  /*
		  line_i[j].values[x] =
		    line_i[j].values[x] - line_k[j].values[x]
		    * ( line_i[k].values[x] / line_k[k].values[x] );
		    */


		  /*
		  assert( i != k );
		  line_i[j].values[x] =
		      line_i[j].values[x] * line_k[k].values[x]
		    - line_k[j].values[x] * line_i[k].values[x];
		  */
		}
	    }
	  simulation->cells[i][k].state = CELL_ZERO;
#ifdef USE_OMP
#pragma omp parallel for
#endif
	  for ( x = 0 ; x < s ; ++x )
	    {
	      simulation->cells[i][k].values[x]=0.L;
	    }
	}

    }

  //simulation_dump(simulation);
  //backward elimination
  //simulation_dump(simulation);

  if (1)
  for ( k = m - 1 ; k >= 0 ; --k )
    {
      //set pivot to 1
      // Lk = Lk / Fk,k
#ifdef USE_OMP
#pragma omp parallel for
#endif
      for ( j = k+1 ; j < n ; ++j )
	{
	  assert( j != k );
	  simulation->cells[k][j].state = CELL_SET;
	  for ( x = 0 ; x < s ; ++x )
	    {
	      //assert( simulation->cells[k][k].state != CELL_ZERO && !cell_is_zero(simulation, &simulation->cells[k][k]));

	      simulation->cells[k][j].values[x] =
		c_let(
		      c_div(simulation->cells[k][j].values[x], simulation->cells[k][k].values[x] ));
	      /*
	      simulation->cells[k][j].values[x] =
		simulation->cells[k][j].values[x] / simulation->cells[k][k].values[x];
	      */
	    }
	}
      simulation->cells[k][k].state = CELL_POSITIVE_UNITY;
#ifdef USE_OMP
#pragma omp parallel for
#endif
      for ( x = 0 ; x < s ; ++x )
	{
	  simulation->cells[k][k].values[x] = 1.L + I * 0.L;
	}
    }


  //zeroes pivot col
  for ( k = m - 1 ; k >= 0 ; --k ) // vertical pivot Fk,k
    {
      assert( simulation->cells[k][k].state == CELL_POSITIVE_UNITY );
      // Li = Li - Lk * Fik
#ifdef USE_OMP
#pragma omp parallel for
#endif
      for ( i = 0 ; i < k ; ++i) // vertical
	{
	  assert( i != k );
	  for ( j = k + 1 ; j < m ; ++ j )
	    assert( simulation->cells[i][j].state == CELL_ZERO );

	  // => Fir = Fir - Fik x Fkr
	  // => Fi,k  = 0
	  int result_col = n - 1;
	  simulation->cells[i][result_col].state = CELL_SET;
	  for ( x = 0 ; x < s ; ++x )
	    {
	      simulation->cells[i][result_col].values[x] = c_let(
		c_sub(simulation->cells[i][result_col].values[x],
		      c_mul(simulation->cells[i][k].values[x], simulation->cells[k][result_col].values[x])));
	      /*
	      simulation->cells[i][j].values[x] =
		simulation->cells[i][j].values[x]
		- simulation->cells[i][k].values[x] * simulation->cells[k][j].values[x];
	      */
	    }

	  simulation->cells[i][k].state = CELL_ZERO;
	  for ( x = 0 ; x < s ; ++x )
	    {
	      simulation->cells[i][k].values[x] = 0.L;
	    }
	}

    }

  //sanity check
  assert( m == simulation->n_vars );
  assert( n == simulation->n_vars+1 );
  for ( i = 0 ; i < m ; ++ i )
    for ( j = 0 ; j < m ; ++ j )
      if ( i==j )
	assert(simulation->cells[i][j].state == CELL_POSITIVE_UNITY );
      else
	assert(simulation->cells[i][j].state == CELL_ZERO );
}
