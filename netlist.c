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
#include <string.h>
#include <stdio.h>

void netlist_free(netlist_t *netlist)
{
  int i;
  if ( NULL != netlist )
    for ( i = 0 ; i < vec_dipole_count(netlist->dipoles) ; ++i )
      {
	dipole_free(&vec_dipole(netlist->dipoles)[i]);
      }
  vec_dipole_free(netlist->dipoles);
  free(netlist);
}

status_t netlist_new(simulation_context_t *sc, const char *orig_netlist_str, netlist_t **netlistp)
{
  status_t status;
  char *netlist_str = strdup(orig_netlist_str);
  char *tmp1, *tmp2, *line;
  netlist_t *netlist = malloc( sizeof *netlist );

  netlist->sc = sc;

  netlist->dipoles = vec_dipole_new(0);
 
  for ( line = strtok_r(netlist_str, "\n", &tmp1) ;
        line != NULL ;
	line = strtok_r(NULL, "\n", &tmp1) )
    {
      if ( '.' == line[0] ||
	   '*' == line[0] ||
	   '$' == line[0] ||
	   '!' == line[0] )
	continue;
      char *name, *node1 = NULL, *node2 = NULL, *magnitude = NULL, *param1 = NULL, *param2 = NULL;
      name = strtok_r(line, " ", &tmp2);
      if ( NULL != name )
	node1 = strtok_r(NULL, " ", &tmp2);
      if ( NULL != node1 )
	node2 = strtok_r(NULL, " ", &tmp2);
      if ( NULL != node2 )
	magnitude = strtok_r(NULL, " ", &tmp2);
      if ( NULL != magnitude )
	param1 = strtok_r(NULL, " ", &tmp2);
      if ( NULL != param1 )
	param2 = strtok_r(NULL, " ", &tmp2);
      
      if ( NULL == name || NULL == node1 || NULL == node2 || NULL == magnitude )
	{
	  fprintf(stderr, "Missing parameter in netlist\n");
	  status = FAILURE;
	  goto end;
	}

      dipole_t new_dipole;
      status = dipole_new(sc,
			  vec_dipole_count(netlist->dipoles),
			  name,
			  node1,
			  node2,
			  strtod(magnitude, NULL),
			  param1, param2, & new_dipole);
      vec_dipole_push_back(netlist->dipoles, new_dipole);
      if ( SUCCESS != status )
	goto end;
    
    }
 end:
  if ( SUCCESS == status && NULL != netlistp )
    *netlistp = netlist;
  else
    netlist_free(netlist);
  free(netlist_str);
  return status;
}

void netlist_dump(netlist_t *netlist)
{
  int i;
  printf("* Netlist dump *\n");
  for( i = 0 ; i < vec_dipole_count(netlist->dipoles) ; ++i )
    {
      printf("%s\t%s\t%s\t%f\t%s\t%s\n",
	     vec_dipole(netlist->dipoles)[i].name,
	     vec_dipole(netlist->dipoles)[i].node1,
	     vec_dipole(netlist->dipoles)[i].node2,
	     (double)vec_dipole(netlist->dipoles)[i].magnitude,
	     vec_dipole(netlist->dipoles)[i].param1,
	     vec_dipole(netlist->dipoles)[i].param2);
      
    }
}
