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
 * DC policy:
 *   1 -> approximate H(0) with the first simulated sample H(1 Hz)
 *   0 -> force H(0) = 0
 */
#ifndef YANA_IMPULSE_DC_EQUALS_FIRST_SAMPLE
#define YANA_IMPULSE_DC_EQUALS_FIRST_SAMPLE 0
#endif

static inline yana_complex_t
load_bin(const double *z, int i)
{
  return REAL(z, i) + I * IMAG(z, i);
}

static inline void
store_bin(double *z, int i, yana_complex_t v)
{
  REAL(z, i) = creal(v);
  IMAG(z, i) = cimag(v);
}

static void
build_square_wave(yana_complex_t *x, int n_time, double f0)
{
  for (int i = 0; i < n_time; ++i) {
    double s = sin(2.0 * M_PI * f0 * (double)i / (double)n_time);
    x[i] = (s >= 0.0) ? 1.0 : -1.0;
  }
}

/*
 * Causal step over the displayed 1 s window:
 *   x[0] = 0
 *   x[n>=1] = 1
 */
static void
build_step(yana_complex_t *x, int n_time)
{
  x[0] = 0.0;
  for (int i = 1; i < n_time; ++i) {
    x[i] = 1.0;
  }
}

static void
build_impulse(yana_complex_t *x, int n_time)
{
  x[0] = 1.0;
  for (int i = 1; i < n_time; ++i) {
    x[i] = 0.0;
  }
}

static void
build_cis(yana_complex_t *x, int n_time, double f0)
{
  for (int i = 0; i < n_time; ++i) {
    x[i] = cexp(I * 2.0 * M_PI * f0 * (double)i / (double)n_time);
  }
}

status_t
result_impulse(simulation_context_t *sc,
               uforth_output_t *freq_output,
               double fe,
               uforth_output_t **time_outputp)
{
  (void)sc;

  int n_vars;
  int n_freq;
  int n_time;
  int half;

  double *data = NULL;
  double *excitation_fft = NULL;
  yana_complex_t *x = NULL;
  yana_complex_t *time_values = NULL;
  gsl_fft_complex_wavetable *wavetable = NULL;
  gsl_fft_complex_workspace *workspace = NULL;
  uforth_output_t *time_output = NULL;

  if (NULL == time_outputp || NULL == freq_output) {
    return FAILURE;
  }
  *time_outputp = NULL;

  n_vars = uforth_output_columns(freq_output);
  n_freq = uforth_output_lines(freq_output);

  if (n_freq <= 0 || n_vars <= 0) {
    return FAILURE;
  }

  /*
   * freq_output contains the positive-frequency response:
   *   line 0         -> 1 Hz
   *   ...
   *   line n_freq-1  -> n_freq Hz
   *
   * To use all computed frequencies in a real-system reconstruction, build
   * a 2*n_freq-point spectrum:
   *   bin 0                 -> DC
   *   bins 1..n_freq-1      -> +f
   *   bin n_freq            -> Nyquist
   *   bins 2*n_freq-k       -> -f
   *
   * This yields a full 1-second time record with:
   *   Fs = 2*n_freq Hz
   *   dt = 1 / (2*n_freq) s
   */
  n_time = 2 * n_freq;
  half = n_freq;

  data = calloc(2 * n_time, sizeof(double));
  excitation_fft = calloc(2 * n_time, sizeof(double));
  x = calloc(n_time, sizeof(yana_complex_t));
  time_values = calloc((size_t)(n_vars - 1) * (size_t)n_time, sizeof(yana_complex_t));
  wavetable = gsl_fft_complex_wavetable_alloc(n_time);
  workspace = gsl_fft_complex_workspace_alloc(n_time);

  if (NULL == data || NULL == excitation_fft || NULL == x ||
      NULL == time_values || NULL == wavetable || NULL == workspace) {
    free(data);
    free(excitation_fft);
    free(x);
    free(time_values);
    if (wavetable) {
      gsl_fft_complex_wavetable_free(wavetable);
    }
    if (workspace) {
      gsl_fft_complex_workspace_free(workspace);
    }
    return FAILURE;
  }

  if (fe > 0.0) {
    build_square_wave(x, n_time, fe);
  } else if (fe == 0.0) {
    build_impulse(x, n_time);
  } else if (fe == -1.0) {
    build_step(x, n_time);
  } else {
    build_cis(x, n_time, -fe);
  }

  for (int i = 0; i < n_time; ++i) {
    store_bin(data, i, x[i]);
  }

  gsl_fft_complex_forward(data, 1, n_time, wavetable, workspace);
  memcpy(excitation_fft, data, 2 * n_time * sizeof(double));

  for (int idx = 1; idx < n_vars; ++idx) {
    memset(data, 0, 2 * n_time * sizeof(double));

    yana_complex_t h0 =
      YANA_IMPULSE_DC_EQUALS_FIRST_SAMPLE
      ? uforth_output_value(freq_output, 0, idx)
      : 0.0;

    /* DC */
    store_bin(data, 0, load_bin(excitation_fft, 0) * h0);

    /*
     * Positive-frequency bins:
     *   freq_output line (k - 1) -> FFT bin k -> k Hz
     */
    for (int k = 1; k < half; ++k) {
      yana_complex_t hk = uforth_output_value(freq_output, k - 1, idx);
      yana_complex_t yk = load_bin(excitation_fft, k) * hk;

      store_bin(data, k, yk);
      store_bin(data, n_time - k,
                load_bin(excitation_fft, n_time - k) * conj(hk));
    }

    /*
     * Nyquist bin:
     *   freq_output line (n_freq - 1) -> FFT bin n_freq
     *
     * Keep only the real part here so the reconstructed waveform stays
     * real-valued when the spectrum is intended to represent a real signal.
     */
    {
      yana_complex_t hnyq = uforth_output_value(freq_output, n_freq - 1, idx);
      yana_complex_t ynyq = load_bin(excitation_fft, half) * hnyq;
      store_bin(data, half, creal(ynyq));
    }

    gsl_fft_complex_inverse(data, 1, n_time, wavetable, workspace);

    for (int i = 0; i < n_time; ++i) {
      time_values[(size_t)(idx - 1) * (size_t)n_time + (size_t)i] =
        load_bin(data, i);
    }
  }

  time_output = uforth_output_new();
  if (NULL == time_output) {
    free(data);
    free(excitation_fft);
    free(x);
    free(time_values);
    gsl_fft_complex_wavetable_free(wavetable);
    gsl_fft_complex_workspace_free(workspace);
    return FAILURE;
  }

  for (int i = 0; i < n_time; ++i) {
    uforth_output_dot(time_output, (yana_real_t)i / (yana_real_t)n_time);

    for (int idx = 1; idx < n_vars; ++idx) {
      uforth_output_dot(
        time_output,
        time_values[(size_t)(idx - 1) * (size_t)n_time + (size_t)i]
      );
    }

    uforth_output_newline(time_output);
  }

  *time_outputp = time_output;

  gsl_fft_complex_wavetable_free(wavetable);
  gsl_fft_complex_workspace_free(workspace);
  free(x);
  free(time_values);
  free(excitation_fft);
  free(data);

  return SUCCESS;
}
