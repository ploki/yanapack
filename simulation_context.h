#ifndef __YANA_SC_H__
#define __YANA_SC_H__ 1

typedef struct simulation_context_t
{
  int log_hz_min;
  int log_hz_max;
  int samples_per_decade;
} simulation_context_t;


void simulation_context_free(simulation_context_t *sc);
simulation_context_t *
simulation_context_new(int log_hz_min, int log_hz_max, int samples_per_decade);
int simulation_context_get_n_samples(simulation_context_t *sc);
yana_real_t simulation_context_get_f(simulation_context_t *sc, int i);

int simulation_context_get_sample(simulation_context_t *sc, yana_real_t f);

#endif
