#ifndef __YANA_DIPOLE_H__
#define __YANA_DIPOLE_H__ 1

typedef enum
  {
    YANA_UNKNOWN,
    YANA_RESISTOR,
    YANA_INDUCTOR,
    YANA_CAPACITOR,
    YANA_TRANSFORMER,
    YANA_GYRATOR,
    YANA_CURRENT_SOURCE,
    YANA_TENSION_SOURCE,

    YANA_RADIATOR,
    YANA_PORT,
    YANA_BOX,
    YANA_FREE_AIR,

  } dipole_type_t;

typedef struct dipole_t 
{
  simulation_context_t *sc;
  int idx;
  dipole_type_t type;
  int quadripole_done;
  char *name;
  char *node1;
  char *node2;
  yana_real_t magnitude;
  char *param1;
  char *param2;
  yana_complex_t *values;
} dipole_t;

void dipole_free(dipole_t *dipole);
status_t dipole_new(simulation_context_t *sc,
		    int idx,
		    const char *name,
		    const char *node1,
		    const char *node2,
		    yana_real_t magnitude,
		    const char *param1,
		    const char *param2,
		    dipole_t *dipole);
yana_complex_t free_air_impedance(yana_real_t f, yana_real_t r, yana_real_t solid_angle_div_by_pi);


#endif
