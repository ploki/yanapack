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
#ifndef __YANA_NETLIST_H__
#define __YANA_NETLIST_H__ 1
#include <yana_vector.h>

/*
[R]idx node1 noded2 magnitude maxpower
[L]idx node1 node2 magnitude resistance maxpower
[C]idx node1 node2 magnitude maxvolt
[T][LR]idx node1 node2 magnitude
[G][LR]idx node1 node2 magnitude
[EV]idx node1 node2 magnitude
[I]idx node1 node2 magnitude

[Z]idx node1 node2 surface [ma][ai] (impedance de radiation)
[P]idx node1 node2 length surface kk ( kk: correction factor: k:free air K:flanged)
[B]idx node1 node2 volume
[A]idx node1 node2 distance solid_angle_div_by_pi [ai] ( source to listener impedance  )
*/

USE_VEC_T(dipole)

typedef struct netlist_t
{
  simulation_context_t *sc;
  VEC_T(dipole) *dipoles;
} netlist_t;

void netlist_free(netlist_t *netlist);
status_t netlist_new(simulation_context_t *sc, const char *orig_netlist_str, netlist_t **netlistp);
void netlist_dump(netlist_t *netlist, const char* filter);

#endif
