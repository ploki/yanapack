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

static void
build_square_wave(yana_complex_t *square_wave, int n_samples, double fe) {
  const int order = n_samples;
  const double f0 = fe ;
  double sin_lut[n_samples];
#pragma omp parallel for
  for (int i = 0; i < n_samples; ++i) {
    double x = (double)i  / (double)n_samples;
    sin_lut[i] = sin(2. * M_PI * f0 * x);
  }
#pragma omp parallel for
  for(int i = 0; i < n_samples; ++i) {
    //double x = (double)i  / (double)n_samples;
    square_wave[i] = -.5;
    for(int n = 1 ; n < order; n+=2) {
      //if (f0 * n > n_samples/2) break;
      //if (n > 10) break;
      //or no break, complete spectrum
      square_wave[i] += 4. / ( M_PI * n) * sin_lut[(n * i) % n_samples];//
    }
  }
}

void simulation_impulse(simulation_context_t *sc,
                        simulation_t *simulation,
                        double fe)
{
  /**
   * fe:
   *     0 -> Dirac Distribution
   *     1 -> Heaviside Distribution
   *    -1 -> Sign Distribution? no...
   *     2 -> Exponential decay? no...
   */
  int n_vars = simulation->n_vars;
  int n_samples = simulation_context_get_n_samples(sc);
  double *data = calloc(2 * n_samples, sizeof(double));
  gsl_fft_complex_wavetable * wavetable = gsl_fft_complex_wavetable_alloc (n_samples);
  gsl_fft_complex_workspace * workspace = gsl_fft_complex_workspace_alloc (n_samples);

  yana_complex_t square_wave[n_samples];

  const yana_real_t fe_squarewave_threshold = 2; //blind under 2Hz.

  if (abs(fe) >= fe_squarewave_threshold) {
    build_square_wave(square_wave, n_samples, fe);
    for (int i = 0; i < n_samples/2; ++i)
      {
        REAL(data, i) = creal(square_wave[i * 2]);
        IMAG(data, i) = cimag(square_wave[i * 2]);
        REAL(data, n_samples - i - 1) = creal(square_wave[i * 2] + 1);
        IMAG(data, n_samples - i - 1) = cimag(square_wave[i * 2] + 1);
      }
    if (1)
      gsl_fft_complex_forward(data, 1, n_samples,
                              wavetable, workspace);
    for(int i = 0; i < n_samples / 2; ++i) {
      square_wave[i * 2] = (I * IMAG(data, i) + REAL(data, i));
      square_wave[i * 2 + 1] = (I * IMAG(data, n_samples - i - 1)
                                + REAL(data, n_samples - i - 1));
    }
  }

  yana_real_t sign = fe >= 0 ? 1 : -1;
  for (int idx = 0; idx < n_vars; ++idx) {
    yana_complex_t *result = simulation->cells[idx][simulation->n_vars].values;
    result[0]=0;
    for (int i = 0; i < n_samples/2; ++i)
      {
        if (fe < fe_squarewave_threshold) {
          yana_complex_t
            w_even = I*simulation_context_get_f(sc, i * 2 + 0),
            w_odd  = I*simulation_context_get_f(sc, i * 2 + 1),
            f_even = cpow(w_even, abs(fe)),
            f_odd = cpow(w_odd, abs(fe)),
            even = result[i * 2]     * f_even,
            odd  = result[i * 2 + 1] * f_odd;
          REAL(data, i) = creal(even);
          IMAG(data, i) = cimag(even);
          REAL(data, n_samples - i - 1) = creal(odd * sign);
          IMAG(data, n_samples - i - 1) = cimag(odd * sign);
        } else {
          yana_complex_t
            f_even = square_wave[i * 2],
            f_odd  = square_wave[i * 2 + 1],
            even = f_even * result[i * 2],
            odd  = f_odd * result[i * 2 + 1];
          REAL(data, i) = creal(even);
          IMAG(data, i) = cimag(even);
          REAL(data, n_samples - i - 1) = creal(odd);
          IMAG(data, n_samples - i - 1) = cimag(odd);
        }
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


yana_complex_t
H_low_pass(yana_complex_t p, yana_complex_t w0, int order)
{
  return cpow(1. / (1 + p / w0), order);
}

yana_complex_t
H_high_pass(yana_complex_t p, yana_complex_t w0, int order)
{
  return cpow(1. / (1 + w0 / p), order);
}

void
apply_window_function(yana_complex_t *samples,
                      int n_samples,
                      yana_real_t delta_f,
                      //yana_real_t lf,
                      yana_real_t hf,
                      yana_real_t order)
{
#pragma omp parallel for
  for (int i = 0; i < n_samples; ++i) {
    samples[i] = samples[i]
      //* H_high_pass(I * 2. * M_PI * i * delta_f, 2. * M_PI * lf, order)
      * H_low_pass(I * 2. * M_PI * i * delta_f, 2. * M_PI * hf, order);
  }
}



void result_impulse(simulation_context_t *sc,
                    uforth_output_t *output,
                    double fe)
{
  /**
   * fe:
   *     0 -> Dirac Distribution
   *     1 -> Heaviside Distribution
   *    -1 -> Sign Distribution? no...
   *     2 -> Exponential decay? no...
   */
  int n_vars = uforth_output_columns(output);
  int n_samples = uforth_output_lines(output);
  double *data = calloc(2 * n_samples, sizeof(double));
  gsl_fft_complex_wavetable * wavetable = gsl_fft_complex_wavetable_alloc (n_samples);
  gsl_fft_complex_workspace * workspace = gsl_fft_complex_workspace_alloc (n_samples);

  yana_complex_t square_wave[n_samples];
  if (abs(fe) >= 2) {
    build_square_wave(square_wave, n_samples, fe);
    for (int i = 0; i < n_samples/2; ++i)
      {
        REAL(data, i) = creal(square_wave[i * 2]);
        IMAG(data, i) = cimag(square_wave[i * 2]);
        REAL(data, n_samples - i - 1) = creal(square_wave[i * 2] + 1);
        IMAG(data, n_samples - i - 1) = cimag(square_wave[i * 2] + 1);
      }
    if (1)
      gsl_fft_complex_forward(data, 1, n_samples,
                              wavetable, workspace);
    for(int i = 0; i < n_samples / 2; ++i) {
      square_wave[i * 2] = (I * IMAG(data, i) + REAL(data, i));
      square_wave[i * 2 + 1] = (I * IMAG(data, n_samples - i - 1)
                                + REAL(data, n_samples - i - 1));
    }
  }

  yana_real_t sign = fe >= 0 ? 1 : -1;
  // don't transform the first, it's by convention the frequency
  for (int idx = 1; idx < n_vars; ++idx) {
    yana_complex_t result[n_samples];
    for (int i = 0; i < n_samples; ++i) {
      result[i] = uforth_output_value(output, i, idx);
    }

    apply_window_function(result, n_samples, 1, n_samples/2, 8);

    for (int i = 0; i < n_samples/2; ++i)
      {
        if (fe < 2) {
          yana_complex_t
            w_even = I*simulation_context_get_f(sc, i * 2 + 0),
            w_odd  = I*simulation_context_get_f(sc, i * 2 + 1),
            f_even = cpow(w_even, abs(fe)),
            f_odd = cpow(w_odd, abs(fe)),
            even = result[i * 2]     * f_even,
            odd  = result[i * 2 + 1] * f_odd;
          REAL(data, i) = creal(even);
          IMAG(data, i) = cimag(even);
          REAL(data, n_samples - i - 1) = creal(odd * sign);
          IMAG(data, n_samples - i - 1) = cimag(odd * sign);
        } else {
          yana_complex_t
            f_even = square_wave[i * 2],
            f_odd  = square_wave[i * 2 + 1],
            even = f_even * result[i * 2],
            odd  = f_odd * result[i * 2 + 1];
          REAL(data, i) = creal(even);
          IMAG(data, i) = cimag(even);
          REAL(data, n_samples - i - 1) = creal(odd);
          IMAG(data, n_samples - i - 1) = cimag(odd);
        }
      }
    if(1)
      gsl_fft_complex_inverse(data, 1, n_samples,
                              wavetable, workspace);

    for(int i = 0; i < n_samples / 2; ++i) {
      uforth_output_set(output, i * 2, idx, (I * IMAG(data, i) + REAL(data, i)));
      uforth_output_set(output, i * 2 + 1, idx, (I * IMAG(data, n_samples - i - 1)
                                                 + REAL(data, n_samples - i - 1)));
    }
  }

  //inverse axis
  for (int i = 0; i < n_samples; ++i) {
    yana_complex_t freq = uforth_output_value(output, i, 0);
    //fprintf(stderr, "freq=%f, => %f\n", cabs(freq), cabs(freq/(double)n_samples));
    uforth_output_set(output, i, 0, freq/(double)n_samples);
  }

  gsl_fft_complex_wavetable_free (wavetable);
  gsl_fft_complex_workspace_free (workspace);
  free(data);
}
