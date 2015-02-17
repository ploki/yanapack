/* 
 * Copyright (c) 2013-2015, Guillaume Gimenez <guillaume@blackmilk.fr>
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
#define _GNU_SOURCE
#include <uforth.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <assert.h>
#include <ctype.h>

typedef enum
  {
    UF_FREEAIR, // ( r1 r2 -- c )
    UF_MUL,     // ( c1 c2 -- c ) mul c1 by c2
    UF_DIV,     // ( c1 c2 -- c ) div c1 by c2
    UF_ADD,     // ( c1 c2 -- c ) add c1 to c2
    UF_SUB,     // ( c1 c2 -- c ) sub c2 from c1
    UF_NEG,     // ( c1 -- c ) -c1
    UF_EXP,     // ( c1 -- c ) e^c1
    UF_POW,     // ( c1 c2 -- c ) c1^c2
    UF_LN,      // ( c1 -- c ) ln(c1)
    UF_LOG,     // ( c1 -- c ) ln(c1)/ln(10)
    UF_ABS,     // ( c1 -- r ) modulus of c1
    UF_ARG,     // ( c1 -- r ) argument of c1
    UF_DEG,     // ( r -- r ) rad to deg
    UF_IMAG,    // ( c1 -- r ) imag part of c1
    UF_REAL,    // ( c1 -- r ) real part of c1
    UF_PI,      // ( -- r ) pi
    UF_F,       // ( -- r ) current freq
    UF_S,       // ( -- r ) current step
    UF_I,       // ( -- i ) push i into the stack
    UF_DB,      // ( c -- r ) convert to dB
    UF_DBSPL,      // ( c -- r ) convert to dBSPL
    UF_DOT,     // ( v -- ) print
    UF_DUP,     // ( v -- v v ) dup
    UF_VALUE_REAL,
    UF_VALUE_COMPLEX,
    UF_VALUE_SIMULATION,
  } uforth_token_type_t;

typedef struct uforth_token
{
  uforth_token_type_t type;
  yana_real_t r;
  yana_complex_t c;
  char *symbol;
  struct uforth_token *next;
} uforth_token_t;

struct uforth_context
{
  uforth_token_t *first;
  uforth_token_t **last;
  int stack_size;
  int stack_pos;
  uforth_token_t stack[];
};

uforth_token_t *
uforth_token_new(uforth_token_type_t type,
		 const char *symbol,
		 yana_real_t r,
		 yana_complex_t c)
{
  uforth_token_t *token = calloc(1, sizeof *token);
  assert( NULL != token );
  token->type = type;
  token->r = r;
  token->symbol = symbol?strdup(symbol):NULL;
  token->c = c;
  token->next = NULL;
  return token;
}

status_t uforth_compile(const char *buf, int stack_size,
			uforth_context_t **uf_ctxp)
{
  uforth_context_t *uf_ctx;
  uf_ctx = malloc(sizeof(uforth_context_t) +
		  sizeof(uforth_token_t) * stack_size);
  assert( NULL != uf_ctx );
  uf_ctx->stack_size = stack_size;
  uf_ctx->stack_pos = 0;
  uf_ctx->first = NULL;
  uf_ctx->last = &uf_ctx->first;
  char *code_str = strdup(buf);
  char *tmp_l, *tmp_k, *line, *token;
  for ( line = strtok_r(code_str, "\n", &tmp_l) ;
	line != NULL ;
	line = strtok_r(NULL, "\n", &tmp_l ))
    {
      if ( '.' != line[0] )
	continue;
      ++line;
      for ( token = strtok_r(line, " ", &tmp_k) ;
	    token != NULL ;
	    token = strtok_r(NULL, " ", &tmp_k))
	{
	  char *symbol = NULL;
	  yana_real_t r = 0.;
	  uforth_token_type_t token_type;
	  if ( 0 == strcasecmp(token, "FREEAIR") )
	    token_type = UF_FREEAIR;
	  else if ( 0 == strcasecmp(token, "MUL") ||
		    0 == strcmp(token, "*") )
	    token_type = UF_MUL;
	  else if ( 0 == strcasecmp(token, "DIV") ||
		    0 == strcmp(token, "/") )
	    token_type = UF_DIV;
	  else if ( 0 == strcasecmp(token, "ADD")  ||
		    0 == strcmp(token, "+") )
	    token_type = UF_ADD;
	  else if ( 0 == strcasecmp(token, "SUB") ||
		    0 == strcmp(token, "-") )
	    token_type = UF_SUB;
	  else if ( 0 == strcasecmp(token, "NEG") )
	    token_type = UF_NEG;
	  else if ( 0 == strcasecmp(token, "EXP") ||
		    0 == strcmp(token, "e") )
	    token_type = UF_EXP;
	  else if ( 0 == strcasecmp(token, "POW") )
	    token_type = UF_POW;
	  else if ( 0 == strcasecmp(token, "LN") )
	    token_type = UF_LN;
	  else if ( 0 == strcasecmp(token, "LOG") )
	    token_type = UF_LOG;
	  else if ( 0 == strcasecmp(token, "ABS") )
	    token_type = UF_ABS;
	  else if ( 0 == strcasecmp(token, "ARG") )
	    token_type = UF_ARG;
	  else if ( 0 == strcasecmp(token, "DEG") )
	    token_type = UF_DEG;
	  else if ( 0 == strcasecmp(token, "IMAG") )
	    token_type = UF_IMAG;
	  else if ( 0 == strcasecmp(token, "REAL") )
	    token_type = UF_REAL;
	  else if ( 0 == strcasecmp(token, "PI") )
	    token_type = UF_PI;
	  else if ( 0 == strcasecmp(token, "F") )
	    token_type = UF_F;
	  else if ( 0 == strcasecmp(token, "S") )
	    token_type = UF_S;
	  else if ( 0 == strcasecmp(token, "I") )
	    token_type = UF_I;
	  else if ( 0 == strcasecmp(token, "DB") )
	    token_type = UF_DB;
	  else if ( 0 == strcasecmp(token, "DBSPL") )
	    token_type = UF_DBSPL;
	  else if ( 0 == strcasecmp(token, "DOT") ||
		    0 == strcmp(token, ".") )
	    token_type = UF_DOT;
	  else if ( 0 == strcasecmp(token, "DUP") )
	    token_type = UF_DUP;
	  else
	    {
	      char *endptr;
	      r = strtod(token, &endptr);
	      if ( NULL == endptr || *endptr == '\0' )
		{
		  token_type = UF_VALUE_REAL;
		}
	      else
		{
		  token_type = UF_VALUE_SIMULATION;
		  r = 0.;
		  symbol = token;
		}
	    }
	  uforth_token_t *new_token = uforth_token_new(token_type, symbol, r, 0.);
	  *uf_ctx->last = new_token;
	  uf_ctx->last = &new_token->next;
	}
    }
  *uf_ctxp = uf_ctx;
  return SUCCESS;
}

static status_t
pop_real(uforth_context_t *uf_ctx, yana_real_t *rp)
{
  int pos = uf_ctx->stack_pos-1;
  yana_complex_t c;
  if ( uf_ctx->stack_pos <= 0 )
    {
      fprintf(stderr, "ERROR: stack underflow\n");
      return FAILURE;
    }
  
  switch (uf_ctx->stack[pos].type)
    {
    case UF_VALUE_REAL:
      *rp = uf_ctx->stack[pos].r;
      break;
    case UF_VALUE_COMPLEX:
      c = uf_ctx->stack[pos].c;
      if ( abs(cimag(c)) > 0.000001 )
	{
	  fprintf(stderr, "ERROR: stack element is complex, real was expected\n");
	  return FAILURE;
	}
      *rp = creal(c);
      break;
    default:
      fprintf(stderr, "ERROR: unexpected type in stack, real was expected\n");
      return FAILURE;
    }
  uf_ctx->stack_pos = pos;
  return SUCCESS;
}
static status_t
pop_complex(uforth_context_t *uf_ctx, yana_complex_t *cp)
{
  int pos = uf_ctx->stack_pos-1;
  if ( uf_ctx->stack_pos <= 0 )
    {
      fprintf(stderr, "ERROR: stack underflow\n");
      return FAILURE;
    }
  
  switch (uf_ctx->stack[pos].type)
    {
    case UF_VALUE_REAL:
      *cp = uf_ctx->stack[pos].r + I * 0.;
      break;
    case UF_VALUE_COMPLEX:
      *cp = uf_ctx->stack[pos].c;
      break;
    default:
      fprintf(stderr, "ERROR: unexpected type in stack, real was expected\n");
      return FAILURE;
    }
  uf_ctx->stack_pos = pos;
  return SUCCESS;
}

static status_t
push_real(uforth_context_t *uf_ctx, yana_real_t r)
{
  int pos = uf_ctx->stack_pos;
  if ( pos+1 >= uf_ctx->stack_size )
    {
      fprintf(stderr, "ERROR: stack overflow\n");
      return FAILURE;
    }
  uf_ctx->stack[pos].type = UF_VALUE_REAL;
  uf_ctx->stack[pos].c = 0;
  uf_ctx->stack[pos].r = r;
  uf_ctx->stack_pos = pos + 1;
  return SUCCESS;
}

static status_t
push_complex(uforth_context_t *uf_ctx, yana_complex_t c)
{
  int pos = uf_ctx->stack_pos;
  if ( pos+1 >= uf_ctx->stack_size )
    {
      fprintf(stderr, "ERROR: stack overflow\n");
      return FAILURE;
    }
  uf_ctx->stack[pos].type = UF_VALUE_COMPLEX;
  uf_ctx->stack[pos].c = c;
  uf_ctx->stack[pos].r = 0;
  uf_ctx->stack_pos = pos + 1;
  return SUCCESS;
}

#define POP_REAL(_op_, _var_)						\
  do {									\
    status_t status = pop_real(uf_ctx, &(_var_));			\
    if ( SUCCESS != status )						\
      {									\
	fprintf(stderr, "ERROR: "_op_" expects a real value\n");	\
	return status;							\
      }									\
  } while (0)
#define POP_COMPLEX(_op_, _var_)					\
  do {									\
    status_t status = pop_complex(uf_ctx, &(_var_));			\
    if ( SUCCESS != status )						\
      {									\
	fprintf(stderr, "ERROR: "_op_" expects a complex value\n");	\
	return status;							\
      }									\
  } while (0)

#define PUSH_REAL(_op_, _val_)						\
  do {									\
    status_t status = push_real(uf_ctx, (_val_));			\
    if ( SUCCESS != status )						\
      {									\
	fprintf(stderr, "ERROR: on "_op_);				\
	return status;							\
      }									\
  } while (0)

#define PUSH_COMPLEX(_op_, _val_)					\
  do {									\
    status_t status = push_complex(uf_ctx, (_val_));			\
    if ( SUCCESS != status )						\
      {									\
	fprintf(stderr, "ERROR: on "_op_);				\
	return status;							\
      }									\
  } while (0)

#define HEAD_TYPE(_var_)				\
  do {							\
    if ( uf_ctx->stack_pos < 1 )			\
      {							\
	fprintf(stderr, "ERROR: stack underflow\n");	\
	return FAILURE;					\
      }							\
    (_var_)=uf_ctx->stack[uf_ctx->stack_pos-1].type;	\
  } while (0)

static status_t
uforth_execute_step(uforth_context_t *uf_ctx,
		    simulation_context_t *sc,
		    simulation_t *simulation,
		    int i)
{
  uforth_token_t *token;
  yana_real_t r1, r2;
  yana_complex_t c1, c2;
  yana_real_t f = simulation_context_get_f(sc, i);
  uforth_token_type_t head_type;
  for ( token = uf_ctx->first ;
	token ;
	token = token->next )
    {
      switch (token->type)
	{
	case UF_FREEAIR:
	  POP_REAL("( x X -- x ) FREEAIR", r2);
	  POP_REAL("( X x -- x ) FREEAIR", r1);
	  PUSH_COMPLEX("FREEAIR", free_air_impedance(f, r1, r2));
	  break;
	case UF_MUL:
	  POP_COMPLEX("( x X -- x ) MUL", c2);
	  POP_COMPLEX("( X x -- x ) MUL", c1);
	  PUSH_COMPLEX("MUL", c1*c2);
	  break;
	case UF_DIV:
	  POP_COMPLEX("( x X -- x ) DIV", c2);
	  POP_COMPLEX("( X x -- x ) DIV", c1);
	  PUSH_COMPLEX("DIV", c1/c2);
	  break;
	case UF_ADD:
	  POP_COMPLEX("( x X -- x ) ADD", c2);
	  POP_COMPLEX("( X x -- x ) ADD", c1);
	  PUSH_COMPLEX("ADD", c1+c2);
	  break;
	case UF_SUB:
	  POP_COMPLEX("( x X -- x ) SUB", c2);
	  POP_COMPLEX("( X x -- x ) SUB", c1);
	  PUSH_COMPLEX("SUB", c1-c2);
	  break;
	case UF_NEG:
	  POP_COMPLEX("( X -- x ) NEG", c1);
	  PUSH_COMPLEX("NEG", -c1);
	  break;
	case UF_EXP:
	  POP_COMPLEX("( X -- x ) EXP", c1);
	  PUSH_COMPLEX("EXP", cexp(c1));
	  break;
	case UF_POW:
	  POP_COMPLEX("( x X -- x ) POW", c2);
	  POP_COMPLEX("( X x -- x ) POW", c1);
	  PUSH_COMPLEX("POW", cpow(c1, c2));
	  break;
	case UF_LN:
	  POP_COMPLEX("( X -- x ) LN", c1);
	  PUSH_COMPLEX("LN", clog(c1));
	  break;
	case UF_LOG:
	  POP_COMPLEX("( X -- x ) LOG", c1);
	  PUSH_COMPLEX("LOG", clog10(c1));
	  break;
	case UF_ABS:
	  POP_COMPLEX("( X -- x ) ABS", c1);
	  PUSH_REAL("ABS", cabs(c1));
	  break;
	case UF_ARG:
	  POP_COMPLEX("( X -- x ) ARG", c1);
	  PUSH_REAL("ARG", carg(c1));
	  break;
	case UF_DEG:
	  POP_REAL("( X -- x ) DEG", r1);
	  PUSH_REAL("DEG", 180. * r1 / M_PI );
	  break;
	case UF_IMAG:
	  POP_COMPLEX("( X -- x ) IMAG", c1);
	  PUSH_REAL("IMAG", cimag(c1));
	  break;
	case UF_REAL:
	  POP_COMPLEX("( X -- x ) REAL", c1);
	  PUSH_REAL("REAL", creal(c1));
	  break;
	case UF_PI:
	  PUSH_REAL("PI", M_PI);
	  break;
	case UF_F:
	  PUSH_REAL("F", f);
	  break;
	case UF_S:
	  PUSH_REAL("F", i);
	  break;
	case UF_I:
	  PUSH_COMPLEX("I", 0. + I * 1.);
	  break;
	case UF_DB:
	  POP_COMPLEX("( X -- x ) DB", c1);
	  PUSH_REAL("DB", 20. * log10(cabs(c1)));
	  break;
	case UF_DBSPL:
	  POP_COMPLEX("( X -- x ) DB", c1);
	  PUSH_REAL("DB", 20. * log10(cabs(c1)/20e-6));
	  break;
	case UF_DOT:
	  HEAD_TYPE(head_type);
	  if ( UF_VALUE_REAL == head_type )
	    {
	      POP_REAL("( X -- ) DOT", r1);
	      fprintf(stdout, "%f\t", (double)r1);
	    }
	  else if ( UF_VALUE_COMPLEX == head_type )
	    {
	      POP_COMPLEX("( X -- ) DOT", c1);
	      r1 = creal(c1);
	      r2 = cimag(c1);
	      if ( abs(r2) < 0.000001 )
		fprintf(stdout, "%f\t", (double)r1);
	      else
		fprintf(stdout, "%f%+fi\t", (double)r1, (double)r2);
	    }
	  break;
	case UF_DUP:
	  POP_COMPLEX("( X -- x x ) DUP", c1);
	  PUSH_COMPLEX("DUP", c1);
	  PUSH_COMPLEX("DUP", c1);
	  break;
	case UF_VALUE_REAL:
	  PUSH_REAL("real lit", token->r);
	  break;
	case UF_VALUE_COMPLEX:
	  assert(!"not possible");
	  PUSH_REAL("complex lit", token->c);
	  break;
	case UF_VALUE_SIMULATION:
	  {
	    yana_complex_t *sim_array;
	    if ( ( token->symbol[0] != 'v' || !isdigit(token->symbol[1] ) ) &&
		 ( token->symbol[0] != 'I' || isdigit(token->symbol[1] ) ) )
	      {
		fprintf(stderr, "ERROR: syntax error on symbol '%s'\n", token->symbol);
		fprintf(stderr, "HINT: dipoles start with 'I' and nodes start with 'v'\n");
		return FAILURE;
	      }
	    sim_array = simulation_result(simulation, token->symbol+1);
	    if ( NULL == sim_array )
	      {
		fprintf(stderr, "ERROR: Unknown symbol '%s'\n", token->symbol);
		return FAILURE;
	      }
	    PUSH_COMPLEX("sim", sim_array[i]);
	  }
	  break;
	}
    }
  fprintf(stdout, "\n");
  if ( uf_ctx->stack_pos != 0 )
    {
      fprintf(stderr, "WARNING: stack not empty at the end of the processing\n");
      uf_ctx->stack_pos = 0;
    }
  return SUCCESS;
}

status_t
uforth_execute(uforth_context_t *uf_ctx,
	       simulation_context_t *sc,
	       simulation_t *simulation)
{
  int i, s;
  status_t status;
  s = simulation_context_get_n_samples(sc);
  for ( i = 0 ; i < s ; ++i )
    {
      status = uforth_execute_step(uf_ctx, sc, simulation, i);
      if ( SUCCESS != status )
	{
	  fprintf(stderr, "Execution failed\n");
	  return status;
	}
    }
  return SUCCESS;
}
