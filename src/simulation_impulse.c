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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_fft_complex.h>

#define REAL(z,i) ((z)[2*(i)])
#define IMAG(z,i) ((z)[2*(i)+1])

/*
 * Set to 1 to approximate H(0) with the first simulated sample H(1 Hz).
 * Set to 0 to force H(0) = 0.
 */
#ifndef YANA_IMPULSE_DC_EQUALS_FIRST_SAMPLE
#define YANA_IMPULSE_DC_EQUALS_FIRST_SAMPLE 1
#endif

static inline yana_complex_t
load_bin(const double *data, int i)
{
  return REAL(data, i) + I * IMAG(data, i);
}

static inline void
store_bin(double *data, int i, yana_complex_t z)
{
  REAL(data, i) = creal(z);
  IMAG(data, i) = cimag(z);
}

static yana_complex_t
build_square_wave(yana_complex_t *square_wave, int n_samples, double fe) {
  const int order = n_samples;
  const double f0 = fe;
  double sin_lut[n_samples];
#pragma omp parallel for
  for (int i = 0; i < n_samples; ++i) {
    double x = (double)i / (double)n_samples;
    sin_lut[i] = sin(2. * M_PI * x);
  }
#pragma omp parallel for
  for (int i = 0; i < n_samples; ++i) {
    square_wave[i] = 0;
    for (int n = 1; n < order; n += 2) {
      square_wave[i] += 4. / (M_PI * n) * sin_lut[(int)(f0 * n * i) % n_samples];
    }
  }
  return 0;
}

static yana_complex_t
build_step(yana_complex_t *distribution, int n_samples)
{
  distribution[0] = 0;
  for (int i = 1; i < n_samples; ++i) {
    distribution[i] = 1;
  }
  return 1;
}

static yana_complex_t
build_impulse(yana_complex_t *distribution, int n_samples)
{
  distribution[0] = 1;
  for (int i = 1; i < n_samples; ++i) {
    distribution[i] = 0;
  }
  return 1;
}

static yana_complex_t
build_cis(yana_complex_t *distribution, int n_samples, double fe)
{
  yana_complex_t dc = 0;
  for (int i = 0; i < n_samples; ++i) {
    dc += (distribution[i] = -cexp(I * M_PI * (1. / 2. + 2 * ((double)i / n_samples) * fe)));
  }
  return dc;
}

void simulation_impulse(simulation_context_t *sc,
                        simulation_t *simulation,
                        double fe)
{
  int n_vars = simulation->n_vars;
  int n_samples = simulation_context_get_n_samples(sc);
  int half = n_samples / 2;

  double *data = NULL;
  double *excitation_fft = NULL;
  gsl_fft_complex_wavetable *wavetable = NULL;
  gsl_fft_complex_workspace *workspace = NULL;

  yana_complex_t the_wave[n_samples];

  if (n_samples < 2 || (n_samples % 2) != 0)
    return;

  data = calloc(2 * n_samples, sizeof(double));
  excitation_fft = calloc(2 * n_samples, sizeof(double));
  wavetable = gsl_fft_complex_wavetable_alloc(n_samples);
  workspace = gsl_fft_complex_workspace_alloc(n_samples);

  if (NULL == data || NULL == excitation_fft || NULL == wavetable || NULL == workspace) {
    free(data);
    free(excitation_fft);
    if (wavetable)
      gsl_fft_complex_wavetable_free(wavetable);
    if (workspace)
      gsl_fft_complex_workspace_free(workspace);
    return;
  }

  if (fe > 0)
    build_square_wave(the_wave, n_samples, fe / 4.);
  else if (fe == 0)
    build_impulse(the_wave, n_samples);
  else if (fe == -1)
    build_step(the_wave, n_samples);
  else if (fe < -1)
    build_cis(the_wave, n_samples, -fe / 4.);
  else
    memset(the_wave, 0, sizeof(the_wave));

  /* FFT of the excitation signal */
  for (int i = 0; i < n_samples; ++i)
    store_bin(data, i, the_wave[i]);

  gsl_fft_complex_forward(data, 1, n_samples, wavetable, workspace);
  memcpy(excitation_fft, data, 2 * n_samples * sizeof(double));

  for (int idx = 0; idx < n_vars; ++idx) {
    yana_complex_t *result = simulation->cells[idx][simulation->n_vars].values;
    yana_complex_t h0 =
      YANA_IMPULSE_DC_EQUALS_FIRST_SAMPLE ? result[0] : 0;

    memset(data, 0, 2 * n_samples * sizeof(double));

    /* DC bin */
    store_bin(data, 0, load_bin(excitation_fft, 0) * h0);

    /*
     * Positive-frequency bins:
     *   result[0]      -> 1 Hz  -> FFT bin 1
     *   result[1]      -> 2 Hz  -> FFT bin 2
     *   ...
     *   result[half-1] -> N/2Hz -> FFT bin N/2
     *
     * We rebuild the negative side explicitly with conjugate symmetry.
     */
    for (int k = 1; k < half; ++k) {
      yana_complex_t yk = load_bin(excitation_fft, k) * result[k - 1];
      store_bin(data, k, yk);
      store_bin(data, n_samples - k, conj(yk));
    }

    /*
     * Nyquist bin is self-conjugate, so keep it real to preserve a real
     * time-domain signal.
     */
    {
      yana_complex_t y_nyquist = load_bin(excitation_fft, half) * result[half - 1];
      store_bin(data, half, creal(y_nyquist));
    }

    gsl_fft_complex_inverse(data, 1, n_samples, wavetable, workspace);

    for (int i = 0; i < n_samples; ++i)
      result[i] = load_bin(data, i);
  }

  gsl_fft_complex_wavetable_free(wavetable);
  gsl_fft_complex_workspace_free(workspace);
  free(excitation_fft);
  free(data);
}
