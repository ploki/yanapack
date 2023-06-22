/*
 * Copyright (c) 2013-2019, Guillaume Gimenez <guillaume@blackmilk.fr>
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
#include "simulation_impulse.h"
#include <stdio.h>
#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_fft_complex.h>

#define REAL(z,i) ((z)[2*(i)])
#define IMAG(z,i) ((z)[2*(i)+1])

static yana_complex_t
build_square_wave(yana_complex_t *square_wave, int n_samples, double fe) {
  const int order = n_samples;
  const double f0 = fe ;
  double sin_lut[n_samples];
#pragma omp parallel for
  for (int i = 0; i < n_samples; ++i) {
    double x = (double)i  / (double)n_samples;
    sin_lut[i] = sin(2. * M_PI * x);
  }
#pragma omp parallel for
  for(int i = 0; i < n_samples; ++i) {
    //double x = (double)i  / (double)n_samples;
    square_wave[i] = 0;
    for(int n = 1 ; n < order; n+=2) {
      square_wave[i] += 4. / ( M_PI * n) * sin_lut[(int)(f0 * n * i) % n_samples];//
    }
  }
  return 0;
}

static yana_complex_t
build_step(yana_complex_t *distribution, int n_samples)
{
  distribution[0] = 0;
  for (int i = 1; i < n_samples; ++i) {
    distribution[i] = n_samples-1;
  }
  return n_samples-1;
}

static yana_complex_t
build_impulse(yana_complex_t *distribution, int n_samples)
{
  distribution[0] = 1;
  for (int i = 1; i < n_samples; ++i) {
    distribution[i] = 0;
  }
  return 0*-1./(double)(n_samples-1);
}
static yana_complex_t
build_cis(yana_complex_t *distribution, int n_samples, double fe)
{
  yana_complex_t dc = 0;
  //distribution[0]=0;
  for (int i = 0; i < n_samples; ++i) {
    dc += (distribution[i] = -cexp( I * M_PI * ( 1./2. + 2*((double)i/n_samples)*fe) ));
  }
  return dc;
}

void simulation_impulse(simulation_context_t *sc,
                        simulation_t *simulation,
                        double fe)
{
  int n_vars = simulation->n_vars;
  int n_samples = simulation_context_get_n_samples(sc);
  double *data = calloc(2 * n_samples, sizeof(double));
  gsl_fft_complex_wavetable * wavetable = gsl_fft_complex_wavetable_alloc (n_samples);
  gsl_fft_complex_workspace * workspace = gsl_fft_complex_workspace_alloc (n_samples);

  yana_complex_t the_wave[n_samples];
  yana_complex_t dc;

  if (fe > 0 )
    dc = build_square_wave(the_wave, n_samples, fe/4.);
  else if (fe == 0)
    dc = build_impulse(the_wave, n_samples);
  else if (fe == -1)
    dc = build_step(the_wave, n_samples);
  else if (fe < -1)
    dc = build_cis(the_wave, n_samples, -fe/4.);
  else dc = 0;

  for (int i = 0; i < n_samples/2; ++i)
    {
      REAL(data, i) = creal(the_wave[i * 2]);
      IMAG(data, i) = cimag(the_wave[i * 2]);
      REAL(data, n_samples - i - 1) = creal(the_wave[i * 2 + 1]);
      IMAG(data, n_samples - i - 1) = cimag(the_wave[i * 2 + 1]);
    }
  if (1)
    gsl_fft_complex_forward(data, 1, n_samples,
                            wavetable, workspace);
  for(int i = 0; i < n_samples / 2; ++i) {
    the_wave[i * 2] = (    REAL(data, i)
                              + I * IMAG(data, i));
    the_wave[i * 2 + 1] = (REAL(data, n_samples - i - 1)
                              + I * IMAG(data, n_samples - i - 1));
  }

  for (int idx = 0; idx < n_vars; ++idx) {
    yana_complex_t *result = simulation->cells[idx][simulation->n_vars].values;
    for (int i = 0; i < n_samples/2; ++i)
      {
        yana_complex_t
          f_even = the_wave[i * 2],
          f_odd  = the_wave[i * 2 + 1],
          even = f_even * (i == 0 ? dc : result[i * 2 - 1]),
          odd  = f_odd * result[i * 2];
        REAL(data, i) = creal(even);
        IMAG(data, i) = cimag(even);
        REAL(data, n_samples - i - 1) = creal(odd);
        IMAG(data, n_samples - i - 1) = cimag(odd);
      }
    if(1)
      gsl_fft_complex_inverse(data, 1, n_samples,
                              wavetable, workspace);

    for(int i = 0; i < n_samples / 2; ++i) {
      result[i * 2] = (I * IMAG(data, i) + REAL(data, i));
      result[i * 2 + 1] = (I * IMAG(data, n_samples - i - 1)
                           + REAL(data, n_samples - i - 1));
    }
  }

  gsl_fft_complex_wavetable_free (wavetable);
  gsl_fft_complex_workspace_free (workspace);
  free(data);
}
