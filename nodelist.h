#ifndef __YANA_NODELIST_H__
#define __YANA_NODELIST_H__ 1

typedef int int_t;

USE_VEC_T(int)

typedef struct node_t
{
  VEC_T(int) *connections;
  int n_connections;
  int is_gnd;
  char *name;
} node_t;

USE_VEC_T(node)

typedef struct nodelist_t
{
  netlist_t *netlist;
  int n_dipoles;
  VEC_T(node) *nodes;
  yana_map_t *names;
} nodelist_t;

status_t nodelist_new(simulation_context_t *sc, netlist_t *netlist, nodelist_t **nodelistp);
void nodelist_free(nodelist_t *nodelist);
void nodelist_dump(nodelist_t *nodelist);
int nodelist_idx(nodelist_t *nodelist, const char *node_name);


#endif
