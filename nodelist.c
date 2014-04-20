#include <yanapack.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


static node_t *node_new(nodelist_t *nodelist, const char *node_name, int *idxp)
{
  int i;
  node_t node;
  node.is_gnd = 0;
  node.n_connections = 0;
  node.name = strdup(node_name);
  node.connections = vec_int_new(nodelist->n_dipoles);
  for ( i = 0 ; i < nodelist->n_dipoles ; ++i )
    vec_int_push_back(node.connections, 0);
  if ( NULL != idxp )
    *idxp = vec_node_count(nodelist->nodes);
  return vec_node_push_back(nodelist->nodes, node);
}

static int *intdup(int i)
{
  int *res = malloc(sizeof(*res));
  *res = i;
  return res;
}

static node_t * node_get(nodelist_t *nodelist, const char *node_name)
{
  const yana_pair_t *pair = yana_map_get(nodelist->names, node_name);
  node_t *node;
  int idx = 0;
  if ( NULL == pair )
    {
      node = node_new(nodelist, node_name, &idx);
      yana_map_set(nodelist->names,
		   yana_pair_new(node->name,
				 intdup(idx)));
    }
  else
    {
      idx = *(int*)pair->value;
      node = &vec_node(nodelist->nodes)[idx];
    }
  return node;
}

int nodelist_idx(nodelist_t *nodelist, const char *node_name)
{
  const yana_pair_t *pair = yana_map_get(nodelist->names, node_name);
  if ( NULL == pair )
    assert(!"Unexpected node name");
  return *(int*)pair->value;
}

static void node_free(node_t *node)
{
  vec_int_free(node->connections);
  free(node->name);
}

void nodelist_free(nodelist_t *nodelist)
{
  int i;
  netlist_free(nodelist->netlist);
  for ( i = 0 ; i < vec_node_count(nodelist->nodes) ; ++i )
    {
      node_free(&vec_node(nodelist->nodes)[i]);
    }
  vec_node_free(nodelist->nodes);
  yana_map_free(nodelist->names);
  free(nodelist);
}

static void
nodelist_set_connection(nodelist_t *nodelist, int dipole, const char *node_name, int sign)
{
  node_t *node = node_get(nodelist, node_name);
  assert( 0 == vec_int(node->connections)[dipole] );
  vec_int(node->connections)[dipole] = sign;
  ++node->n_connections;
}

static int name_cmp(const yana_pair_t *lhs, const yana_pair_t *rhs)
{
  return strcmp((const char*)lhs->key,(const char*)rhs->key);
}
static void name_free(yana_pair_t *pair)
{
  free(pair->value);
  free(pair);
}

status_t nodelist_new(simulation_context_t *sc, netlist_t *netlist, nodelist_t **nodelistp)
{
  int n_dipoles = vec_dipole_count(netlist->dipoles);
  nodelist_t *nodelist = malloc( sizeof *nodelist );
  int i;
  
  nodelist->n_dipoles = n_dipoles;
  nodelist->netlist = netlist;

  nodelist->nodes = vec_node_new(1);
  nodelist->names = yana_map_new(name_cmp, name_free);

  for ( i = 0 ; i < n_dipoles ; ++i )
    {
      nodelist_set_connection(nodelist, i, vec_dipole(netlist->dipoles)[i].node1, 1);
      nodelist_set_connection(nodelist, i, vec_dipole(netlist->dipoles)[i].node2, -1);
    }
  for ( i = 0 ; i < vec_node_count(nodelist->nodes) ; ++i )
    {
      if ( vec_node(nodelist->nodes)[i].n_connections < 2 )
	{
	  fprintf(stderr," node %d connected to only %d dipoles\n",i,vec_node(nodelist->nodes)[i].n_connections);
	  assert(0);
	}
    }

  *nodelistp=nodelist;
  return SUCCESS;
}
void node_dump(node_t *node, int n_dipoles)
{
  int i;
  for ( i = 0 ; i < n_dipoles ; ++i )
    {
      if ( 0 == vec_int(node->connections)[i] )
	printf("\t");
      else
	printf("%+d\t",vec_int(node->connections)[i]);
    }
  printf("\n");
}

void nodelist_dump(nodelist_t *nodelist)
{
  int i;
  printf("* Nodelist *\ndps:\t");
  for ( i = 0 ; i < vec_dipole_count(nodelist->netlist->dipoles) ; i++ )
    {
      printf("%s\t",vec_dipole(nodelist->netlist->dipoles)[i].name);
    }
  printf("\n");
  for ( i = 0 ; i < vec_node_count(nodelist->nodes) ; ++i )
    {
      printf("n%d:\t",i);
      node_dump(&vec_node(nodelist->nodes)[i], nodelist->n_dipoles);
    }
}
