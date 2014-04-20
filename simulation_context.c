#include <stdlib.h>

#include <yanapack.h>


void simulation_context_free(simulation_context_t *sc)
{
  free(sc);
}

simulation_context_t *
simulation_context_new(int log_hz_min, int log_hz_max, int samples_per_decade)
{
  simulation_context_t *sc = malloc ( sizeof *sc);
  sc->log_hz_min = log_hz_min;
  sc->log_hz_max = log_hz_max;
  sc->samples_per_decade = samples_per_decade;
  return sc;
}

int simulation_context_get_n_samples(simulation_context_t *sc)
{
  return 1 + ( sc->log_hz_max - sc->log_hz_min ) * sc->samples_per_decade ;
}

yana_real_t simulation_context_get_f(simulation_context_t *sc, int i)
{
  if ( 0 == sc->samples_per_decade )
    return pow(10.L, (yana_real_t)sc->log_hz_min * i );
  else
    return pow(10.L, (yana_real_t)sc->log_hz_min
	       + (yana_real_t)i / (yana_real_t)sc->samples_per_decade);
}

int simulation_context_get_sample(simulation_context_t *sc, yana_real_t f)
{
  int sample = (log10(f)-(yana_real_t)sc->log_hz_min) * sc->samples_per_decade;
  if ( sample < 0 )
    return -1;
  else if ( sample >= simulation_context_get_n_samples(sc) )
    return -1;
  else
    return sample;
}
