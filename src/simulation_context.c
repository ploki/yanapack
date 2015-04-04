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
#include <stdlib.h>

#include <yanapack.h>


void simulation_context_free(simulation_context_t *sc)
{
  free(sc);
}

simulation_context_t *
simulation_context_new(int log_hz_min, int log_hz_max, int samples_per_decade)
{
  simulation_context_t *sc = malloc ( sizeof *sc);
  sc->log_hz_min = log_hz_min;
  sc->log_hz_max = log_hz_max;
  sc->samples_per_decade = samples_per_decade;
  return sc;
}

int simulation_context_get_n_samples(simulation_context_t *sc)
{
  return 1 + ( sc->log_hz_max - sc->log_hz_min ) * sc->samples_per_decade ;
}

yana_real_t simulation_context_get_f(simulation_context_t *sc, int i)
{
  if ( 0 == sc->samples_per_decade )
    return pow(10.L, (yana_real_t)sc->log_hz_min * i );
  else
    return pow(10.L, (yana_real_t)sc->log_hz_min
	       + (yana_real_t)i / (yana_real_t)sc->samples_per_decade);
}

int simulation_context_get_sample(simulation_context_t *sc, yana_real_t f)
{
  int sample = (log10(f)-(yana_real_t)sc->log_hz_min) * sc->samples_per_decade;
  if ( sample < 0 )
    return -1;
  else if ( sample >= simulation_context_get_n_samples(sc) )
    return -1;
  else
    return sample;
}
