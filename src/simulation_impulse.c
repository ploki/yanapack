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
  const int order = n_samples/2;
  double sin_lut[n_samples];
#pragma omp parallel for
  for (int i = 0; i < n_samples; ++i) {
    double x = (double)i  / (double)n_samples;
    sin_lut[i] = sin(2. * M_PI * x);
  }
#pragma omp parallel for
  for(int i = 0; i < n_samples; ++i) {
    square_wave[i] = 0;
    for(int n = 1 ; n < order; n+=2) {
      int idx = (int)(fe * n * i) % n_samples;
      square_wave[i] += 4. * sin_lut[idx] / ( M_PI * n) ;
    }
  }
  return 0;
}

static yana_complex_t
build_step(yana_complex_t *distribution, int n_samples)
{
  yana_complex_t dc = build_square_wave(distribution, n_samples, 1);
  for(int i = 0; i < n_samples; ++i) {
    distribution[i] = 1./2.-distribution[i]/2;
  }
  return dc;
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
  for (int i = 0; i < n_samples; ++i) {
    dc += (distribution[i] = -cexp( I * M_PI * ( 1./2. + 2*((double)i/n_samples)*fe) ));
  }
  return dc;
}

void result_impulse(simulation_context_t *sc,
                    uforth_output_t *output,
                    double fe)
{
    int n_vars    = uforth_output_columns(output);
    int n_samples = uforth_output_lines(output);
    double *data = calloc(2 * n_samples, sizeof(double));
    gsl_fft_complex_wavetable * wavetable = gsl_fft_complex_wavetable_alloc(n_samples);
    gsl_fft_complex_workspace * workspace = gsl_fft_complex_workspace_alloc(n_samples);

    yana_complex_t the_wave[n_samples];
    __attribute__((unused)) yana_complex_t dc;

    if (fe > 0)
      dc = build_square_wave(the_wave, n_samples, fe);
    else if (fe == 0)
      dc = build_impulse(the_wave, n_samples);
    else if (fe == -1)
      dc = build_step(the_wave, n_samples);
    else if (fe < -1)
      dc = build_cis(the_wave, n_samples, -fe);
    else
      dc = 0;

    for (int i = 0; i < n_samples; i++) {
        REAL(data, i) = creal(the_wave[i]);
        IMAG(data, i) = cimag(the_wave[i]);
    }

    gsl_fft_complex_forward(data, 1, n_samples,
                            wavetable, workspace);

    for (int i = 0; i < n_samples; i++) {
        the_wave[i] = REAL(data, i) + I * IMAG(data, i);
    }

    // don't transform the first, it's by convention the frequency int the output
    for (int idx = 1; idx < n_vars; idx++)
      {
      for (int i = 0; i < n_samples; i++) {
        yana_complex_t product = i == 0
          ? 0.
          : the_wave[i] * uforth_output_value(output, i-1, idx);
        REAL(data, i) = creal(product);
        IMAG(data, i) = cimag(product);
      }

      gsl_fft_complex_inverse(data, 1, n_samples, wavetable, workspace);

      for (int i = 0; i < n_samples; i++) {
        yana_complex_t time_val = REAL(data, i) + I * IMAG(data, i);
        uforth_output_set(output, i, idx, time_val);
      }
    }

    gsl_fft_complex_wavetable_free(wavetable);
    gsl_fft_complex_workspace_free(workspace);
    free(data);
}
