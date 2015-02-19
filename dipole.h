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
#ifndef __YANA_DIPOLE_H__
#define __YANA_DIPOLE_H__ 1

typedef enum
  {
    YANA_UNKNOWN,
    YANA_RESISTOR,
    YANA_INDUCTOR,
    YANA_CAPACITOR,
    YANA_TRANSFORMER,
    YANA_GYRATOR,
    YANA_CURRENT_SOURCE,
    YANA_TENSION_SOURCE,

    YANA_RADIATOR,
    YANA_PORT,
    YANA_BOX,
    YANA_FREE_AIR,

  } dipole_type_t;

typedef struct dipole_t 
{
  simulation_context_t *sc;
  int idx;
  dipole_type_t type;
  int quadripole_done;
  char *name;
  char *node1;
  char *node2;
  yana_real_t magnitude;
  char *param1;
  char *param2;
  yana_complex_t *values;
} dipole_t;

void dipole_free(dipole_t *dipole);
status_t dipole_new(simulation_context_t *sc,
		    int idx,
		    const char *name,
		    const char *node1,
		    const char *node2,
		    yana_real_t magnitude,
		    const char *param1,
		    const char *param2,
		    dipole_t *dipole);
yana_complex_t free_air_impedance(yana_real_t f, yana_real_t r, yana_real_t solid_angle_div_by_pi);
yana_real_t
dipole_parse_magnitude(const char *str);


#endif
