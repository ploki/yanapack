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
#include <assert.h>
#include <gsl/gsl_sf_bessel.h>


dipole_type_t dipole_name_to_type(const char *name)
{
  switch (name[0])
    {
    case 'r': case 'R':
      return YANA_RESISTOR;
    case 'l': case 'L':
      return YANA_INDUCTOR;
    case 'c': case 'C':
      return YANA_CAPACITOR;
    case 't': case 'T':
      return YANA_TRANSFORMER;
    case 'g': case 'G':
      return YANA_GYRATOR;
    case 'i': case 'I':
      return YANA_CURRENT_SOURCE;
    case 'e': case 'E': case 'v': case 'V':
      return YANA_TENSION_SOURCE;
    case 'z': case 'Z':
      return YANA_RADIATOR;
    case 'p': case 'P':
      return YANA_PORT;
    case 'b': case 'B':
      return YANA_BOX;
    case 'a': case 'A':
      return YANA_FREE_AIR;
    default:
      return YANA_UNKNOWN;
    }
}


void dipole_free(dipole_t *dipole)
{
  if ( NULL != dipole )
    {
      free(dipole->name);
      free(dipole->node1);
      free(dipole->node2);
      free(dipole->values);
      free(dipole->param1);
      free(dipole->param2);
    }
}

status_t dipole_init_simple(dipole_t *dipole)
{
  int i, s;
  for ( i = 0, s = simulation_context_get_n_samples(dipole->sc) ;
	i < s ;
	++ i )
    {
      dipole->values[i] = dipole->magnitude;
    }
  return SUCCESS;
}

status_t dipole_init_gyrator(dipole_t *dipole)
{
  int i, s;
  for ( i = 0, s = simulation_context_get_n_samples(dipole->sc) ;
	i < s ;
	++ i )
    {
      dipole->values[i] = 1.L / dipole->magnitude;
    }
  return SUCCESS;
}

status_t dipole_init_inductor(dipole_t *dipole)
{
  int i, s;
  yana_real_t series_resistor = 0.L;
  if ( dipole->param1 )
    series_resistor = dipole_parse_magnitude(dipole->param1);

  for ( i = 0, s = simulation_context_get_n_samples(dipole->sc) ;
	i < s ;
	++ i )
    {
      dipole->values[i] = series_resistor + I * 2.L * M_PI * simulation_context_get_f(dipole->sc, i) * dipole->magnitude;
    }
  return SUCCESS;
}

status_t dipole_init_capacitor(dipole_t *dipole)
{
  int i, s;
  for ( i = 0, s = simulation_context_get_n_samples(dipole->sc) ;
	i < s ;
	++ i )
    {
      dipole->values[i] = -I / ( 2.L * M_PI * simulation_context_get_f(dipole->sc, i) * dipole->magnitude );
    }
  return SUCCESS;
}

yana_real_t H1(yana_real_t z)
{
  return (2.L / M_PI)
    - gsl_sf_bessel_J0(z)
    + ( 16.L / M_PI - 5.L ) * sin(z)/z
    + (12.L - 36.L/M_PI) * ( 1.L - cos(z))/(z*z);
}

yana_complex_t piston_radiator(yana_real_t ka, yana_real_t magnitude)
{
  yana_complex_t imp = 
    (
     magnitude * ( 1.L - gsl_sf_bessel_J1 (2.L*ka) / (ka) )
     + I * magnitude * H1(2.L*ka ) / ka
     );
  return imp;
}

status_t dipole_init_radiator(dipole_t *dipole)
{
  //[Z]idx node1 node2 surface [ma][ai] (impedance de radiation)
  int i, s;
  yana_real_t surface = dipole->magnitude;
  enum {
    impedance,
    admitance
  } analogy = impedance;
  enum {
    mechanical,
    acoustical
  } realm = acoustical;
    
  if ( NULL != dipole->param1 )
    {
      if ( 'm' == dipole->param1[0] )
	realm = mechanical;
      else if ( 'a' == dipole->param1[0] )
	realm = acoustical;
      if ( 'i' == dipole->param1[1] )
	analogy = impedance;
      else if ( 'a' == dipole->param1[1] )
	analogy = admitance;
    }
  /*
  fprintf(stderr,"piston radiator: realm:%s, analogy:%s\n",
	  realm==mechanical?"mechanical":"acoustical",
	  analogy==impedance?"impedance":"admitance");
  */
  yana_real_t magnitude ;
  if ( acoustical == realm )
    magnitude = YANA_RHO * YANA_C / surface ;
  else
    magnitude = YANA_RHO * YANA_C * surface ;

  yana_real_t a = sqrt( surface / M_PI );

  for ( i = 0, s = simulation_context_get_n_samples(dipole->sc) ;
	i < s ;
	++ i )
    {
      yana_real_t k = 2.L*M_PI*simulation_context_get_f(dipole->sc,i) / YANA_C;
      yana_complex_t imp = piston_radiator(k*a, magnitude);
      if ( impedance == analogy )
	dipole->values[i] = imp;
      else
	dipole->values[i] = ( I*0.L+1.L ) / imp;
    }
  return SUCCESS;
}
status_t dipole_init_port(dipole_t *dipole)
{
  //[P]idx node1 node2 length surface kk ( kk: correction factor: k:free air K:flanged)
  int i, s;
  yana_real_t surface = 0;
  yana_real_t k1 = YANA_FREE_AIR_K, k2 = YANA_FLANGED_K, k;
  yana_real_t length;
  yana_real_t actual_length_div_radius;

  if ( dipole->param1 )
    surface = dipole_parse_magnitude(dipole->param1);

  if ( dipole->param2 )
    {
      if ( dipole->param2[0] == 'K' )
	k1 = YANA_FLANGED_K;
      if ( dipole->param2[1] == 'k' )
	k2 = YANA_FREE_AIR_K;
    }
  k = k1 + k2 ;
  length = dipole->magnitude + ( k * sqrt(surface/M_PI) );
  actual_length_div_radius = dipole->magnitude / sqrt(surface/M_PI);
  for ( i = 0, s = simulation_context_get_n_samples(dipole->sc) ;
	i < s ;
	++ i )
    {
      yana_real_t w = 2.L*M_PI*simulation_context_get_f(dipole->sc,i);
      dipole->values[i] =
	( actual_length_div_radius + 1.L ) * sqrt( 2.L * w * YANA_RHO * YANA_MU ) / ( M_PI * surface * surface )
	+ I * ( YANA_RHO * YANA_C * tan( w * length / YANA_C )  / surface );
    }
  return SUCCESS;

}

status_t dipole_init_box(dipole_t *dipole)
{
  //[B]idx node1 node2 volume
  int i, s;
  yana_real_t C = dipole->magnitude / ( YANA_RHO * YANA_C * YANA_C);
  for ( i = 0, s = simulation_context_get_n_samples(dipole->sc) ;
	i < s ;
	++ i )
    {
      dipole->values[i] = -I / ( 2.L * M_PI * simulation_context_get_f(dipole->sc, i) * C );
    }
  return SUCCESS;
}

yana_complex_t free_air_impedance(yana_real_t f, yana_real_t r, yana_real_t solid_angle_div_by_pi)
{
      yana_real_t w = 2.L * M_PI * f;
      yana_real_t mag = w * YANA_RHO / ( solid_angle_div_by_pi * M_PI * r );
      yana_real_t k = w / YANA_C;
      //yana_complex_t imp = mag;
      //yana_complex_t imp = mag * sin(k*r) + I * mag * cos(k*r);
      yana_complex_t imp = I * mag * cexp(I * k * r);
      return imp;
}
status_t dipole_init_free_air(dipole_t *dipole)
{
  //[A]idx node1 node2 distance solid_angle_div_by_pi [ai] ( source to listener impedance  )
  int i, s;
  yana_real_t r = dipole->magnitude;
  yana_real_t solid_angle_div_by_pi = 2.L;
  if ( NULL != dipole->param1 )
    solid_angle_div_by_pi = dipole_parse_magnitude(dipole->param1);

  // = rho * w / ( 2 * pi * r )
  // = rho * f / r
  for ( i = 0, s = simulation_context_get_n_samples(dipole->sc) ;
	i < s ;
	++ i )
    {
      yana_real_t f = simulation_context_get_f(dipole->sc, i) ;
      dipole->values[i] = free_air_impedance(f, r, solid_angle_div_by_pi);
    }
  return SUCCESS;
}

status_t dipole_init_values(dipole_t *dipole)
{
  switch (dipole->type)
    {
    case YANA_RESISTOR:
    case YANA_TRANSFORMER:
    case YANA_CURRENT_SOURCE:
    case YANA_TENSION_SOURCE:
      return dipole_init_simple(dipole);
    case YANA_GYRATOR:
      return dipole_init_gyrator(dipole);
    case YANA_INDUCTOR:
      return dipole_init_inductor(dipole);
    case YANA_CAPACITOR:
      return dipole_init_capacitor(dipole);
    case YANA_RADIATOR:
      return dipole_init_radiator(dipole);
    case YANA_PORT:
      return dipole_init_port(dipole);
    case YANA_BOX:
      return dipole_init_box(dipole);
    case YANA_FREE_AIR:
      return dipole_init_free_air(dipole);
    case YANA_UNKNOWN:
      assert(!"unknown");
    }
  return SUCCESS;
}

status_t dipole_new(simulation_context_t *sc,
		    int idx,
		    const char *name,
		    const char *node1,
		    const char *node2,
		    yana_real_t magnitude,
		    const char *param1,
		    const char *param2,
		    dipole_t *dipole)
{
  status_t status;
  memset(dipole, 0, sizeof *dipole );

  dipole->sc = sc;
  dipole->type = dipole_name_to_type(name);
  if ( YANA_UNKNOWN == dipole->type )
    {
      fprintf(stderr,"unknown dipole type %s\n",name);
      status = FAILURE;
      goto end;
    }
  dipole->idx = idx;
  dipole->name = name?strdup(name):NULL;
  dipole->node1 = strdup(node1);
  dipole->node2 = strdup(node2);
  dipole->magnitude = magnitude;
  dipole->param1 = param1?strdup(param1):NULL;
  dipole->param2 = param2?strdup(param2):NULL;
  dipole->quadripole_done = 0;

  dipole->values = malloc( sizeof *(dipole->values) * simulation_context_get_n_samples(dipole->sc) );

  status = dipole_init_values(dipole);

 end:
  if (SUCCESS != status )
    dipole_free(dipole);
  return status;
}

yana_real_t
dipole_parse_magnitude(const char *str)
{
  char *tmp;
  yana_real_t res;
  res = strtod(str, &tmp);
  if ( NULL == tmp || '\0' == tmp[0] )
    return res;
  else if ( 0 == strcmp(tmp, "p") )
    return res * 1e-12;
  else if ( 0 == strcmp(tmp, "n") )
    return res * 1e-9;
  else if ( 0 == strcmp(tmp, "u") || 0 == strcmp(tmp, "µ") )
    return res * 1e-6;
  else if ( 0 == strcmp(tmp, "m") )
    return res * 1e-3;
  else if ( 0 == strcmp(tmp, "c") )
    return res * 1e-2;
  else if ( 0 == strcmp(tmp, "d") )
    return res * 1e-1;
  else if ( 0 == strcmp(tmp, "k") )
    return res * 1e3;
  else if ( 0 == strcmp(tmp, "M") || 0 == strcmp(tmp, "meg") )
    return res * 1e6;
  else if ( 0 == strcmp(tmp, "G") )
    return res * 1e9;
  else if ( 0 == strcmp(tmp, "T") )
    return res * 1e12;

  // length
  else if ( 0 == strcmp(tmp, "mm") )
    return res / 1000.;
  else if ( 0 == strcmp(tmp, "cm") )
    return res / 100.;
  else if ( 0 == strcmp(tmp, "dm") )
    return res / 10.;

  // surface
  else if ( 0 == strcmp(tmp, "mm^2") || 0 == strcmp(tmp, "mm²") )
    return res / ( 1000. * 1000. );
  else if ( 0 == strcmp(tmp, "cm^2") || 0 == strcmp(tmp, "cm²") )
    return res / ( 100. * 100. );
  else if ( 0 == strcmp(tmp, "dm^2") || 0 == strcmp(tmp, "dm²") )
    return res / ( 10. * 10. );

  // volume
  else if ( 0 == strcmp(tmp, "L") )
    return res * 1e-3;

  else
    {
      fprintf(stderr, "WARNING: failed to parse real value: %s\n", str);
      return res;
    }
}
