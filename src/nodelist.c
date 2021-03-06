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

static status_t
nodelist_set_connection(nodelist_t *nodelist, int dipole, const char *node_name, int sign)
{
  node_t *node = node_get(nodelist, node_name);
  if ( 0 != vec_int(node->connections)[dipole] )
    {
      fprintf(stderr, "ERROR: node %s connection for dipole %s is already made\n",
	      node->name, vec_dipole(nodelist->netlist->dipoles)[dipole].name);
      return FAILURE;
    }
  vec_int(node->connections)[dipole] = sign;
  ++node->n_connections;
  return SUCCESS;
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
  status_t status;
  int n_dipoles = vec_dipole_count(netlist->dipoles);
  nodelist_t *nodelist = malloc( sizeof *nodelist );
  int i;
  
  nodelist->n_dipoles = n_dipoles;
  nodelist->netlist = netlist;

  nodelist->nodes = vec_node_new(1);
  nodelist->names = yana_map_new(name_cmp, name_free);

  for ( i = 0 ; i < n_dipoles ; ++i )
    {
      status = nodelist_set_connection(nodelist, i, vec_dipole(netlist->dipoles)[i].node1, 1);
      if ( SUCCESS != status )
	return status;
      status = nodelist_set_connection(nodelist, i, vec_dipole(netlist->dipoles)[i].node2, -1);
      if ( SUCCESS != status )
	return status;
    }
  for ( i = 0 ; i < vec_node_count(nodelist->nodes) ; ++i )
    {
      if ( vec_node(nodelist->nodes)[i].n_connections < 2 )
	{
	  fprintf(stderr,"ERROR: Node %s connected to only %d dipole\n",vec_node(nodelist->nodes)[i].name,vec_node(nodelist->nodes)[i].n_connections);
	  return FAILURE;
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
      printf("%s:\t", vec_node(nodelist->nodes)[i].name);
      node_dump(&vec_node(nodelist->nodes)[i], nodelist->n_dipoles);
    }
}
