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
#ifndef __YANA_SIMULATION_H__
#define __YANA_SIMULATION_H__ 1


typedef enum
  {
    CELL_UNDEF,
    CELL_NEGATIVE_UNITY,
    CELL_ZERO,
    CELL_POSITIVE_UNITY,
    CELL_SET,
  } cell_state_t;

typedef struct cell_t
{
  char *dipole_name;
  cell_state_t state;
  yana_complex_t *values;
} cell_t;

/* System of linear equations */
/* first are node tensions, second are branch currents */
typedef struct simulation_t
{
  nodelist_t *nodelist;
  int n_vars;
  cell_t **cells;
  yana_map_t *variables;
} simulation_t;

void simulation_dump(simulation_t *simulation);
status_t simulation_new(simulation_context_t *sc, nodelist_t *nodelist, simulation_t **simulationp);
void simulation_free(simulation_t *simulation);
void simulation_set_values(simulation_t *simulation);
yana_real_t cell_mean(simulation_t *simulation, cell_t *lhs);
yana_real_t cell_mean_module(simulation_t *simulation, cell_t *lhs);
int cell_is_zero(simulation_t *simulation, cell_t *cell);
yana_complex_t *simulation_result(simulation_t *simulation, const char *elem);

#endif
