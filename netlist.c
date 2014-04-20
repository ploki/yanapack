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
