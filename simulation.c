#define _GNU_SOURCE 1
#include <yanapack.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <string.h>

yana_real_t cell_mean(simulation_t *simulation, cell_t *lhs)
{
  int s = simulation_context_get_n_samples(simulation->nodelist->netlist->sc);
  int i;
  yana_complex_t sl = 0;
  yana_real_t ml;
  for ( i = 0 ; i < s ; ++i )
    {
      sl += lhs->values[i];
    }
  sl/=(yana_real_t)s;

  //ml = sqrt(pow(creal(sl),2.L)+pow(cimag(sl),2.L));
  ml = creal(sl);
  return ml;
}

yana_real_t cell_mean_module(simulation_t *simulation, cell_t *lhs)
{
  int s = simulation_context_get_n_samples(simulation->nodelist->netlist->sc);
  int i;
  yana_complex_t sl = 0;
  yana_real_t ml;
  for ( i = 0 ; i < s ; ++i )
    {
      sl += lhs->values[i];
    }
  sl/=(yana_real_t)s;

  ml = sqrt(pow(creal(sl),2.L)+pow(cimag(sl),2.L));
  return ml;
}

int cell_is_zero(simulation_t *simulation, cell_t *cell)
{
  int s = simulation_context_get_n_samples(simulation->nodelist->netlist->sc);
  int i;
  /*
  if ( cell->state == CELL_ZERO )
    return 1;
  */
  for ( i = 0 ; i < s ; ++i )
    {
      if ( fabs(creal(cell->values[i])) > 1e-18 ||
	   fabs(cimag(cell->values[i])) > 1e-18)
	return 0;
    }
  return 1;
}



void simulation_free(simulation_t *simulation)
{
  int i,j;
  for (i = 0 ; i < simulation->n_vars ; ++i )
    {
      for ( j = 0 ; j <= simulation->n_vars ; ++j )
	{
	  free(simulation->cells[i][j].dipole_name);
	  free(simulation->cells[i][j].values);
	}
      free(simulation->cells[i]);
    }
  free(simulation->cells);
  nodelist_free(simulation->nodelist);
  yana_map_free(simulation->variables);
  free(simulation);
}

void simulation_dump(simulation_t *simulation)
{
  int j,i;

  printf("* Simulation Matrix *\n");
  for ( i = 0 ; i < vec_node_count(simulation->nodelist->nodes) ; ++i )
    {
      printf("v%s\t\t", vec_node(simulation->nodelist->nodes)[i].name);
    }
  for ( i = 0 ; i < vec_dipole_count(simulation->nodelist->netlist->dipoles) ; ++i )
    {
      printf("I%s\t\t",vec_dipole(simulation->nodelist->netlist->dipoles)[i].name);
    }
  printf("=\n");
  
  for ( j = 0 ; j < simulation->n_vars ; ++j )
    {
      for ( i = 0 ; i < simulation->n_vars + 1 ; ++i )
	{
	  cell_t *cell = & simulation->cells[j][i];
	  if ( i < simulation->n_vars )
	    {
	      switch (cell->state)
		{
		case CELL_UNDEF:
		  assert(!"UNDEF CELL!");
		case CELL_NEGATIVE_UNITY:
		  printf("-1\t\t"); break;
		  break;
		case CELL_ZERO:
		  if ( simulation->n_vars == i )
		    printf("0\t\t");
		  else
		    printf("\t\t");
		  break;
		case CELL_POSITIVE_UNITY:
		  printf("1\t\t"); break;
		  break;
		case CELL_SET:
		  printf("%s\t\t",cell->dipole_name); break;
		}
	    }
	  else
	    {
	      yana_real_t mean = cell_mean(simulation, cell);
	      printf("%0.7f\t",(double)mean);
	    }
	}
      printf("\n");
    }
}

void cell_set_values(cell_t *cell, int s)
{
  int x;
  yana_complex_t v = 0;
  switch(cell->state)
    {
    case CELL_UNDEF:
      assert(!"undef");
    case CELL_NEGATIVE_UNITY:
      v = -1.L;
      break;
    case CELL_ZERO:
      v = 0.L;
      break;
    case CELL_POSITIVE_UNITY:
      v = 1.L;
      break;
    case CELL_SET:
      return;
    default:
      assert(!"NO!");
    }
  for ( x = 0 ; x < s ; ++x )
    cell->values[x] = v;

}

void simulation_set_values(simulation_t *simulation)
{
  int j,i,s;
  s = simulation_context_get_n_samples(simulation->nodelist->netlist->sc);
  for ( j = 0 ; j < simulation->n_vars ; ++j )
    for ( i = 0 ; i < simulation->n_vars+1 ; ++i )
      {
	if ( simulation->cells[j][i].state == CELL_ZERO )
	  assert( cell_is_zero(simulation, &simulation->cells[j][i]) );
	cell_set_values(&simulation->cells[j][i], s);
      }
}

int simulation_line_cmp(const void *lhsp, const void *rhsp, void *ctx)
{
  const cell_t * const *lhs = lhsp;
  const cell_t * const *rhs = rhsp;
  simulation_t *simulation = ctx;
  int l_zeros=0;
  int r_zeros=0;
  int max_col = simulation->n_vars+1;

  while ( (*lhs)[l_zeros].state == CELL_ZERO && l_zeros < max_col )
    ++l_zeros;
  while ( (*rhs)[r_zeros].state == CELL_ZERO && r_zeros < max_col )
    ++r_zeros;

  if ( l_zeros > r_zeros )
    return 1;
  else if ( l_zeros == r_zeros )
    return 0;
  else
    return -1;
}

void simulation_sort(simulation_t *simulation)
{
  qsort_r(simulation->cells, simulation->n_vars, sizeof(cell_t *), simulation_line_cmp, simulation);
}


void simulation_cell_set_with_dipole(simulation_context_t *sc, cell_t *cell, dipole_t *dipole)
{
  cell->state = CELL_SET;
  cell->dipole_name=strdup(dipole->name);
  int i,s;
  for ( i = 0, s = simulation_context_get_n_samples(sc) ;
	i < s ;
	++i )
    {
      cell->values[i] = dipole->values[i];
    }
}

void simulation_cell_set_with_negative_dipole(simulation_context_t *sc, cell_t *cell, dipole_t *dipole)
{
  cell->state = CELL_SET;
  asprintf(&cell->dipole_name,"-%s", dipole->name);
  int i,s;
  for ( i = 0, s = simulation_context_get_n_samples(sc) ;
	i < s ;
	++i )
    {
      cell->values[i] = - dipole->values[i];
    }
}

void simulation_cell_add_dipole(simulation_context_t *sc, cell_t *cell, dipole_t *dipole)
{
  int i,s;
  s = simulation_context_get_n_samples(sc);
  cell_set_values(cell, s);
  cell->state = CELL_SET;
  if ( cell->dipole_name )
    {
      char *tmp = cell->dipole_name;
      asprintf(&cell->dipole_name,"%s+%s",tmp, dipole->name);
      free(tmp);
    }
  else
    asprintf(&cell->dipole_name,"%s",dipole->name);
  for ( i = 0 ;
	i < s ;
	++i )
    {
      cell->values[i] += dipole->values[i];
    }
}

void simulation_cell_sub_dipole(simulation_context_t *sc, cell_t *cell, dipole_t *dipole)
{
  int i,s;
  s = simulation_context_get_n_samples(sc);
  cell_set_values(cell, s);
  cell->state = CELL_SET;
  if ( cell->dipole_name )
    {
      char *tmp = cell->dipole_name;
      asprintf(&cell->dipole_name,"(%s)-(%s)",tmp,dipole->name);
      free(tmp);
    }
  else
    asprintf(&cell->dipole_name,"%s",dipole->name);
  for ( i = 0 ;
	i < s ;
	++i )
    {
      cell->values[i] -= dipole->values[i];
    }
}

status_t simulation_prepare_transformer(simulation_t *simulation, dipole_t *dipole, int *current_line)
{
  status_t status = SUCCESS;
  dipole_t *other_side;
  int found = 0;
  int i;
  cell_t *line = simulation->cells[*current_line];

  for ( i = 0 ; i < vec_dipole_count(simulation->nodelist->netlist->dipoles) ; ++i )
    {
      other_side = &vec_dipole(simulation->nodelist->netlist->dipoles)[i];

      if ( YANA_TRANSFORMER == other_side->type &&
	   dipole->name[1] != other_side->name[1] &&
	   0 == strcmp(dipole->name+2,other_side->name+2) )
	{
	  found = 1;
	  break;
	}

    }
  if ( !found )
    assert(!"incomplete transformer");

  other_side->quadripole_done = 1;
  dipole->quadripole_done = 1;
  
  //fprintf(stderr,"*** T1=%s, T2=%s\n", dipole->name, other_side->name);

  //printf("v%d * T2 - v%d * T2 - v%d * T1 + v%d T1 = 0\n",dipole->node1, dipole->node2, other_side->node1, other_side->node2);

  simulation_cell_set_with_dipole(simulation->nodelist->netlist->sc,
				  & line[nodelist_idx(simulation->nodelist,dipole->node1)],
				  other_side);
  simulation_cell_set_with_negative_dipole(simulation->nodelist->netlist->sc,
					   &line[nodelist_idx(simulation->nodelist,dipole->node2)],
			    other_side);

  simulation_cell_sub_dipole(simulation->nodelist->netlist->sc,
			     &line[nodelist_idx(simulation->nodelist,other_side->node1)],
			     dipole);
  simulation_cell_add_dipole(simulation->nodelist->netlist->sc,
			     &line[nodelist_idx(simulation->nodelist,other_side->node2)],
			    dipole);
  ++(*current_line);
  line = simulation->cells[*current_line];

  //printf("IT1 * T1 + IT2 * T2 = 0\n");

  simulation_cell_set_with_dipole(simulation->nodelist->netlist->sc,
				  &line[vec_node_count(simulation->nodelist->nodes) + dipole->idx],
				  dipole);
  simulation_cell_set_with_dipole(simulation->nodelist->netlist->sc,
				  &line[vec_node_count(simulation->nodelist->nodes) + other_side->idx],
				  other_side);
  return status;
}
status_t simulation_prepare_gyrator(simulation_t *simulation, dipole_t *dipole, int *current_line)
{
  status_t status = SUCCESS;
  dipole_t *other_side;
  int found = 0;
  int i;
  cell_t *line = simulation->cells[*current_line];

  for ( i = 0 ; i < vec_dipole_count(simulation->nodelist->netlist->dipoles) ; ++i )
    {
      other_side = &vec_dipole(simulation->nodelist->netlist->dipoles)[i];

      if ( YANA_GYRATOR == other_side->type &&
	   dipole->name[1] != other_side->name[1] &&
	   0 == strcmp(dipole->name+2,other_side->name+2) )
	{
	  found = 1;
	  break;
	}

    }
  if ( !found )
    assert(!"incomplete gyrator");
  

#if 1
  line[nodelist_idx(simulation->nodelist,dipole->node1)].state = CELL_POSITIVE_UNITY;
  line[nodelist_idx(simulation->nodelist,dipole->node2)].state = CELL_NEGATIVE_UNITY;
#else
  //fprintf(stderr,"*** G1=%s, G2=%s\n", dipole->name, other_side->name);
  fprintf(stderr,"%d *** v%s * %s(%f) - v%s * %s(%f) ", *current_line,
	  dipole->node1, dipole->name, (double)dipole->magnitude,
	  dipole->node2, dipole->name, (double)dipole->magnitude);
  simulation_cell_set_with_dipole(simulation->nodelist->netlist->sc,
			    & line[dipole->node1],
			    dipole);
  simulation_cell_set_with_negative_dipole(simulation->nodelist->netlist->sc,
			    & line[dipole->node2],
			    dipole);
#endif
  dipole->quadripole_done = 1;
  
  if ( other_side->quadripole_done )
    {
#if 1
      simulation_cell_set_with_negative_dipole(simulation->nodelist->netlist->sc,
					       & line[vec_node_count(simulation->nodelist->nodes) + other_side->idx],
					       dipole);
#else
      fprintf(stderr," - I%s(%d) = 0\n", other_side->name,other_side->idx);
      line[vec_node_count(simulation->nodelist->nodes) + other_side->idx].state = CELL_NEGATIVE_UNITY;
#endif
    }
  else
    {
#if 1
      simulation_cell_set_with_dipole(simulation->nodelist->netlist->sc,
				      & line[vec_node_count(simulation->nodelist->nodes) + other_side->idx],
				      dipole);
#else
      fprintf(stderr," + I%s(%d) = 0\n", other_side->name,other_side->idx);
      line[simulation->nodelist->n_nodes + other_side->idx].state = CELL_POSITIVE_UNITY;
#endif
    }

  return status;
}


status_t simulation_ohm(simulation_t *simulation, int *current_line)
{
  status_t status = SUCCESS;
  int i, j;
  for (i = 0 ; i < simulation->nodelist->n_dipoles ; ++i )
    {
      dipole_t *dipole = &vec_dipole(simulation->nodelist->netlist->dipoles)[i];
      cell_t *line = simulation->cells[*current_line];
      assert ( i == dipole->idx );
      switch (dipole->type)
	{
	case YANA_RESISTOR:
	case YANA_INDUCTOR:
	case YANA_CAPACITOR:
	case YANA_RADIATOR:
	case YANA_PORT:
	case YANA_BOX:
	case YANA_FREE_AIR:
	  asprintf(&line[nodelist_idx(simulation->nodelist,dipole->node1)].dipole_name,"1");
	  asprintf(&line[nodelist_idx(simulation->nodelist,dipole->node2)].dipole_name,"-1");
	  line[nodelist_idx(simulation->nodelist,dipole->node1)].state = CELL_POSITIVE_UNITY;
	  line[nodelist_idx(simulation->nodelist,dipole->node2)].state = CELL_NEGATIVE_UNITY;
	  simulation_cell_set_with_negative_dipole(simulation->nodelist->netlist->sc,
						   &line[vec_node_count(simulation->nodelist->nodes)+i],
						   dipole);
	  break;
	case YANA_TENSION_SOURCE:
	  asprintf(&line[nodelist_idx(simulation->nodelist,dipole->node1)].dipole_name,"1");
	  asprintf(&line[nodelist_idx(simulation->nodelist,dipole->node2)].dipole_name,"-1");
	  line[nodelist_idx(simulation->nodelist,dipole->node1)].state = CELL_POSITIVE_UNITY;
	  line[nodelist_idx(simulation->nodelist,dipole->node2)].state = CELL_NEGATIVE_UNITY;
	  simulation_cell_set_with_dipole(simulation->nodelist->netlist->sc,
				    &line[simulation->n_vars],
				    dipole);
	  if ( !dipole->quadripole_done )
	    {
	      ++(*current_line);
	      line = simulation->cells[*current_line];

#if 1
	      // ground is eg-
	      line[nodelist_idx(simulation->nodelist,dipole->node2)].state = CELL_POSITIVE_UNITY;
#else
	      // ground is last node
	      line[vec_node_count(simulation->nodelist->nodes) - 1 ].state = CELL_POSITIVE_UNITY;
#endif
	      vec_node(simulation->nodelist->nodes)[nodelist_idx(simulation->nodelist,dipole->node2)].is_gnd = 1;

	      line[simulation->n_vars].state = CELL_ZERO;
	      for (j = 0 ; j < simulation->nodelist->n_dipoles ; ++j )
		if ( YANA_TENSION_SOURCE == vec_dipole(simulation->nodelist->netlist->dipoles)[j].type )
		  vec_dipole(simulation->nodelist->netlist->dipoles)[j].quadripole_done = 1;
	    }
	  break;
	case YANA_TRANSFORMER:
	  if ( 0 == dipole->quadripole_done )
	    {
	      simulation_prepare_transformer(simulation, dipole, current_line);
	    }
	  else
	    continue;
	  break;
	case YANA_GYRATOR:
	  simulation_prepare_gyrator(simulation, dipole, current_line);
	  break;
	case YANA_CURRENT_SOURCE:
	case YANA_UNKNOWN:
	  assert(!"not implemented");
	}
      ++(*current_line);
    }

  return status;
}

status_t simulation_kcl(simulation_t *simulation, int *current_line)
{
  /* iterate nodelist to find current equations */
  status_t status = SUCCESS;
  int j,i;
  int processed_nodes = 0;
  for ( j = 0 ; j < vec_node_count(simulation->nodelist->nodes) ; ++j )
    {
      if ( vec_node(simulation->nodelist->nodes)[j].is_gnd )
	continue;
      
      for ( i = 0 ; i < simulation->nodelist->n_dipoles ; ++i )
	{
	  int sign = vec_int(vec_node(simulation->nodelist->nodes)[j].connections)[i];
	  cell_state_t state;
	  const char * state_str;
	  if ( 0 == sign )
	    {
	      state = CELL_ZERO;
	      state_str = "0";
	    }
	  else if ( -1 == sign )
	    {
	      state = CELL_NEGATIVE_UNITY;
	      state_str = "-1";
	    }
	  else if ( 1 == sign )
	    {
	      state = CELL_POSITIVE_UNITY;
	      state_str = "1";
	    }
	  else
	    assert(!"Error in nodelist");
	  asprintf(&simulation->cells[*current_line][vec_node_count(simulation->nodelist->nodes) + i].dipole_name,state_str);
	  simulation->cells[*current_line][vec_node_count(simulation->nodelist->nodes) + i].state = state;
	}
      ++processed_nodes;
      ++(*current_line);

      //in the case of doing kcl before ohm, gnd will not be set in the nodelist
      if ( processed_nodes == vec_node_count(simulation->nodelist->nodes) - 1 )
	break;
    }
  assert (processed_nodes == vec_node_count(simulation->nodelist->nodes) - 1 );

  // report all current sources
  for ( i = 0 ; i < vec_dipole_count(simulation->nodelist->netlist->dipoles) ; ++i )
    {
      if ( vec_dipole(simulation->nodelist->netlist->dipoles)[i].type == YANA_CURRENT_SOURCE )
	{
	  asprintf(&simulation->cells[*current_line][vec_node_count(simulation->nodelist->nodes) + 1].dipole_name,"1");
	  simulation->cells[*current_line][vec_node_count(simulation->nodelist->nodes) + 1].state = CELL_POSITIVE_UNITY;
	  simulation_cell_set_with_dipole(simulation->nodelist->netlist->sc, &simulation->cells[*current_line][simulation->n_vars], &vec_dipole(simulation->nodelist->netlist->dipoles)[i]);
	  ++(*current_line);
	}
    }

  return status;
}

static int variable_cmp(const yana_pair_t *lhs, const yana_pair_t *rhs)
{
  return strcmp((const char*)lhs->key,(const char*)rhs->key);
}
static void variable_free(yana_pair_t *pair)
{
  free(pair->key);
  free(pair->value);
  free(pair);
}

static int *intdup(int i)
{
  int *res = malloc(sizeof(*res));
  *res = i;
  return res;
}

static void init_variables(simulation_context_t *sc, simulation_t *simulation)
{
  int i;
  simulation->variables = yana_map_new(variable_cmp, variable_free);
  for ( i = 0 ; i < vec_node_count(simulation->nodelist->nodes) ; ++i )
    {
      yana_map_set(simulation->variables,
		   yana_pair_new(strdup(vec_node(simulation->nodelist->nodes)[i].name),
				 intdup(i)));
    }
  for ( i = 0 ; i < vec_dipole_count(simulation->nodelist->netlist->dipoles) ; ++i )
    {
      yana_map_set(simulation->variables,
		   yana_pair_new(strdup(vec_dipole(simulation->nodelist->netlist->dipoles)[i].name),
				 intdup(vec_node_count(simulation->nodelist->nodes) + i)));
    }
}

status_t simulation_new(simulation_context_t *sc, nodelist_t *nodelist, simulation_t **simulationp)
{
  simulation_t *simulation;
  status_t status = SUCCESS;
  int i,j,k;
  int n_samples = simulation_context_get_n_samples(sc);
  int current_line;

  simulation = malloc(sizeof *simulation);
  memset(simulation, 0, sizeof *simulation);
  simulation->nodelist = nodelist;
  simulation->n_vars = vec_node_count(nodelist->nodes) + nodelist->n_dipoles;
  
  simulation->cells = malloc( simulation->n_vars * sizeof(cell_t *));

  for ( j = 0 ; j < simulation->n_vars ; ++j )
    {
      simulation->cells[j] = malloc( (simulation->n_vars + 1 ) * sizeof(cell_t));
      
      for ( i = 0 ; i < simulation->n_vars + 1 ; ++i )
	{
	  simulation->cells[j][i].dipole_name = NULL;
	  simulation->cells[j][i].state = CELL_ZERO;
	  simulation->cells[j][i].values = malloc( n_samples * sizeof(yana_complex_t));
	  for ( k = 0 ; k < n_samples ; ++k )
	    simulation->cells[j][i].values[k] = 0;
	}
    }

  current_line = 0;

  status = SUCCESS;

  if ( SUCCESS == status )
    status = simulation_ohm(simulation, &current_line);

  if ( SUCCESS == status )
    status = simulation_kcl(simulation, &current_line);

  init_variables(sc, simulation);

  //simulation_sort(simulation);
  
  *simulationp = simulation;
  return status;
}


yana_complex_t *simulation_result(simulation_t *simulation, const char *elem)
{
  const yana_pair_t *pair = yana_map_get(simulation->variables, elem);
  if ( NULL == pair )
    return NULL;
  int idx = *(int*)pair->value;
  return simulation->cells[idx][simulation->n_vars].values;
}
