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
void netlist_dump(netlist_t *netlist);

#endif
