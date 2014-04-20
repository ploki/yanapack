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
