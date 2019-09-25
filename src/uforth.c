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
#include <stdbool.h>
#include <time.h>

#define MSG(type, fmt,...)				\
  do							\
    {							\
  fprintf(stderr, type ": " fmt "\n", ## __VA_ARGS__);	\
    }							\
  while (0)
#define ERROR(fmt,...) MSG("ERROR", fmt, ## __VA_ARGS__)
#define WARNING(fmt,...) MSG("WARNING", fmt, ## __VA_ARGS__)
#define HINT(fmt,...) MSG("HINT", fmt, ## __VA_ARGS__)

typedef enum
  {
    UF_FREEAIR, // ( r1 r2 -- c )
    UF_DIRIMP,  // ( r1 r2 r3 -- c )
    UF_MUL,     // ( c1 c2 -- c ) mul c1 by c2
    UF_DIV,     // ( c1 c2 -- c ) div c1 by c2
    UF_ADD,     // ( c1 c2 -- c ) add c1 to c2
    UF_SUB,     // ( c1 c2 -- c ) sub c2 from c1
    UF_NEG,     // ( c1 -- c ) -c1
    UF_EXP,     // ( c1 -- c ) e^c1
    UF_POW,     // ( c1 c2 -- c ) c1^c2
    UF_SQRT,    // ( c1 -- c ) sqrt(c1)
    UF_LN,      // ( c1 -- c ) ln(c1)
    UF_LOG,     // ( c1 -- c ) ln(c1)/ln(10)
    UF_COS,
    UF_SIN,
    UF_TAN,
    UF_ACOS,
    UF_ASIN,
    UF_ATAN,
    UF_PAR,
    UF_ABS,     // ( c1 -- r ) modulus of c1
    UF_ARG,     // ( c1 -- r ) argument of c1
    UF_ANGLE,     // ( r1 r2 -- r ) angle difference
    UF_DEG,     // ( r -- r ) rad to deg
    UF_PDELAY,   // ( r -- r ) phase to delay
    UF_PREV_STEP, // ( -- ) jump to previous simulation step
    UF_NEXT_STEP, // ( -- ) jump to next simulation step
    UF_IMAG,    // ( c1 -- r ) imag part of c1
    UF_REAL,    // ( c1 -- r ) real part of c1
    UF_PI,      // ( -- r ) pi
    UF_RHO,
    UF_C,
    UF_MU,
    UF_IMPULSE, // ( -- r ) 1 if impulse mode, 0 if frequency mode
    UF_F,       // ( -- r ) current freq
    UF_S,       // ( -- r ) current step
    UF_I,       // ( -- i ) push i into the stack
    UF_DB,      // ( c -- r ) convert to dB
    UF_DBSPL,      // ( c -- r ) convert to dBSPL
    UF_DOT,     // ( v -- ) print
    UF_DUP,     // ( v -- v v ) dup
    UF_TO,      // ( v -- ) set a word in the heap
    UF_SWAP,    // ( v1 v2 -- v2 v1 ) swap head with head-1
    UF_DROP,
    UF_IF,
    UF_ELSE,
    UF_THEN,
    UF_BEGIN,
    UF_WHILE,
    UF_REPEAT,
    UF_UNTIL,
    UF_AGAIN,
    UF_LEAVE,
    UF_DEPTH,
    UF_LT,
    UF_LE,
    UF_EQ,
    UF_NE,
    UF_GE,
    UF_GT,

    UF_VALUE_DB,      // non-lineaire
    UF_VALUE_REAL,    //dimension physique relle
    UF_VALUE_COMPLEX,
    UF_VALUE_SIMULATION,
  } uforth_token_type_t;

typedef struct uforth_token
{
  uforth_token_type_t type;
  yana_complex_t v;
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

USE_VEC_T(yana_complex);
typedef bool bool_t;
USE_VEC_T(bool);

typedef VEC_T(yana_complex) *line_t;
USE_VEC_T(line);

struct uforth_output
{
  int n_columns;
  VEC_T(bool) *isdB;
  line_t current_line;
  VEC_T(line) *lines;
};

uforth_output_t *
uforth_output_new(void)
{
  uforth_output_t *output;
  output = malloc(sizeof(*output));
  output->n_columns = 0;
  output->current_line = vec_yana_complex_new(0);
  output->lines = vec_line_new(0);
  output->isdB = vec_bool_new(0);
  return output;
}

void
uforth_output_free(uforth_output_t *output)
{
  for (int i = 0; i < vec_line_count(output->lines); ++i) {
    line_t line = vec_line(output->lines)[i];
    vec_yana_complex_free(line);
  }
  vec_yana_complex_free(output->current_line);
  vec_line_free(output->lines);
  free(output);
}

static yana_real_t
complex_to_real(bool isdB, yana_complex_t value)
{
  return isdB ? cabs(value) : creal(value);
}

void
uforth_output_print(uforth_output_t *output)
{
  for (int j = 0; j < vec_line_count(output->lines); ++j) {
    line_t line = vec_line(output->lines)[j];
    line_t prev = NULL, next = NULL;

    if (j > 0) {
      prev = vec_line(output->lines)[j + 1];
    }
    if (j < vec_line_count(output->lines) - 1) {
      next = vec_line(output->lines)[j + 1];
    }
    int c = vec_yana_complex_count(line);
    for (int i = 0; i < c; ++i) {
      /* Don't sum the complex with the low pass filter,
       * otherwise it messes up with the phase,
       * the imaginary part is not used, it's the output */
      bool isdB = vec_bool(output->isdB)[i];
      yana_real_t rvalue = complex_to_real(isdB, vec_yana_complex(line)[i]);
      int count = 1;
      if (0 && prev) {
        rvalue += complex_to_real(isdB, vec_yana_complex(prev)[i]);
        ++count;
      }
      if (0 && next) {
        rvalue += complex_to_real(isdB, vec_yana_complex(next)[i]);
        ++count;
      }
      rvalue /= count;
      if (isdB) {
        fprintf(stdout, "%1.12g\t", 20 * log10(rvalue));
      } else {
        fprintf(stdout, "%1.12g\t", rvalue);
      }
    }
    if (c)
      fprintf(stdout, "\n");
  }
}

static void
uforth_output_dot_dB(uforth_output_t *output, yana_complex_t value)
{
  vec_bool_push_back(output->isdB, true);
  vec_yana_complex_push_back(output->current_line, value);
}

static void
uforth_output_dot(uforth_output_t *output, yana_complex_t value)
{
  vec_bool_push_back(output->isdB, false);
  vec_yana_complex_push_back(output->current_line, value);
}

bool
uforth_output_column_is_dB(uforth_output_t *output, int column)
{
  assert(column < vec_bool_count(output->isdB));
  return vec_bool(output->isdB)[column];
}

static void
uforth_output_newline(uforth_output_t *output)
{
  int current_columns = vec_yana_complex_count(output->current_line);
  if (output->n_columns == 0) {
    output->n_columns = current_columns;
  } else {
    assert(current_columns == output->n_columns && "Sparse output table!");
  }
  vec_line_push_back(output->lines, output->current_line);
  output->current_line = vec_yana_complex_new(0);
}

yana_complex_t
uforth_output_value(uforth_output_t *output, int line, int column)
{
  assert(column < output->n_columns);
  assert(line < vec_line_count(output->lines));
  return vec_yana_complex(vec_line(output->lines)[line])[column];
}

void
uforth_output_set(uforth_output_t *output, int line, int column, yana_complex_t value)
{
  assert(column < output->n_columns);
  assert(line < vec_line_count(output->lines));
  vec_yana_complex(vec_line(output->lines)[line])[column] = value;
}

int
uforth_output_columns(uforth_output_t *output)
{
  return output->n_columns;
}

int
uforth_output_lines(uforth_output_t *output)
{
  return vec_line_count(output->lines);
}

struct uforth_heap
{
  yana_map_t *map;
};

uforth_token_t *
uforth_token_new(uforth_token_type_t type,
		 const char *symbol,
		 yana_complex_t c)
{
  uforth_token_t *token = calloc(1, sizeof *token);
  assert( NULL != token );
  token->type = type;
  token->v = c;
  token->symbol = symbol?strdup(symbol):NULL;
  token->next = NULL;
  return token;
}

void
uforth_token_free(uforth_token_t *token)
{
  if ( token )
    free( token->symbol );
  free(token);
}

static void heap_entry_free(yana_pair_t *pair)
{
  uforth_token_free(pair->value);
}
static int heap_entry_compar(const yana_pair_t *lhs,
			      const yana_pair_t *rhs)
{

  return strcmp((const char*)lhs->key, (const char*)rhs->key);
}
uforth_heap_t *
uforth_heap_new(void)
{
  uforth_heap_t *heap = calloc(1, sizeof *heap);
  if ( NULL == heap )
    return NULL;
  heap->map = yana_map_new(heap_entry_compar, heap_entry_free);
  if ( NULL == heap->map )
    {
      free(heap);
      return NULL;
    }
  return heap;
}

const uforth_token_t *
uforth_heap_get(uforth_heap_t *heap, const char *name)
{
  const yana_pair_t *pair = yana_map_get(heap->map, name);
  if ( NULL == pair )
    return NULL;
  return pair->value;
}
void
uforth_heap_set(uforth_heap_t *heap,
                uforth_token_type_t type,
                const char *name,
                yana_complex_t c)
{
  uforth_token_t *token = uforth_token_new(type, name, c);
  yana_pair_t *pair = yana_pair_new(token->symbol, token);
  assert( NULL != pair );
  yana_map_remove(heap->map, token->symbol);
  yana_map_set(heap->map, pair);
}
void
uforth_heap_remove(uforth_heap_t *heap, const char *name)
{
  yana_map_remove(heap->map, name);
}

void
uforth_heap_free(uforth_heap_t *heap)
{
  yana_map_free(heap->map);
  free(heap);
}

static void
token_swap(uforth_token_t *t1, uforth_token_t *t2)
{
  uforth_token_t tmp;
  memcpy(&tmp, t1, sizeof tmp);
  memcpy(t1, t2, sizeof tmp);
  memcpy(t2, &tmp, sizeof tmp);
}

void
uforth_free(uforth_context_t *uf_ctx)
{
  uforth_token_t *token, *next;
  if ( NULL == uf_ctx )
    return;
  for ( token = uf_ctx->first ;
	token ;
	token = next )
    {
      next = token->next;
      uforth_token_free(token);
    }
  free(uf_ctx);
}

static status_t
compile(const char *buf, int stack_size,
	bool search_dot,
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
  char *tmp_l = NULL, *tmp_k = NULL, *line, *token;
  for ( line = strtok_r(code_str, "\n", &tmp_l) ;
	line != NULL ;
	line = strtok_r(NULL, "\n", &tmp_l ))
    {
      if ( search_dot )
	{
	  if ( '.' != line[0] )
	    continue;
	  if ( ' ' != line[1] )
	    continue;
	  ++line;
	}
      for ( token = strtok_r(line, " ", &tmp_k) ;
	    token != NULL ;
	    token = strtok_r(NULL, " ", &tmp_k))
	{
	  yana_real_t r = 0.;
	  uforth_token_type_t token_type;
	  if ( 0 == strcasecmp(token, "FREEAIR") )
	    token_type = UF_FREEAIR;
	  else if ( 0 == strcasecmp(token, "DIRIMP") )
	    token_type = UF_DIRIMP;
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
	  else if ( 0 == strcasecmp(token, "SQRT") )
	    token_type = UF_SQRT;
	  else if ( 0 == strcasecmp(token, "LN") )
	    token_type = UF_LN;
	  else if ( 0 == strcasecmp(token, "COS") )
	    token_type = UF_COS;
	  else if ( 0 == strcasecmp(token, "SIN") )
	    token_type = UF_SIN;
	  else if ( 0 == strcasecmp(token, "TAN") )
	    token_type = UF_TAN;
	  else if ( 0 == strcasecmp(token, "ACOS") )
	    token_type = UF_ACOS;
	  else if ( 0 == strcasecmp(token, "ASIN") )
	    token_type = UF_ASIN;
	  else if ( 0 == strcasecmp(token, "ATAN") )
	    token_type = UF_ATAN;
	  else if ( 0 == strcasecmp(token, "LOG") )
	    token_type = UF_LOG;
	  else if ( 0 == strcmp(token, "//") ||
		    0 == strcasecmp(token, "PAR") )
	    token_type = UF_PAR;
	  else if ( 0 == strcasecmp(token, "ABS") )
	    token_type = UF_ABS;
	  else if ( 0 == strcasecmp(token, "ARG") )
	    token_type = UF_ARG;
	  else if ( 0 == strcasecmp(token, "ANGLE") )
	    token_type = UF_ANGLE;
	  else if ( 0 == strcasecmp(token, "DEG") )
	    token_type = UF_DEG;
	  else if ( 0 == strcasecmp(token, "PDELAY") )
	    token_type = UF_PDELAY;
	  else if ( 0 == strcasecmp(token, "<<<") || 0 == strcasecmp(token, "PREV"))
	    token_type = UF_PREV_STEP;
	  else if ( 0 == strcasecmp(token, ">>>") || 0 == strcasecmp(token, "PREV") )
	    token_type = UF_NEXT_STEP;
	  else if ( 0 == strcasecmp(token, "IMAG") )
	    token_type = UF_IMAG;
	  else if ( 0 == strcasecmp(token, "REAL") )
	    token_type = UF_REAL;
	  else if ( 0 == strcasecmp(token, "PI") )
	    token_type = UF_PI;
	  else if ( 0 == strcasecmp(token, "_RHO") )
	    token_type = UF_RHO;
	  else if ( 0 == strcasecmp(token, "_C") )
	    token_type = UF_C;
	  else if ( 0 == strcasecmp(token, "_MU") )
	    token_type = UF_MU;
	  else if ( 0 == strcasecmp(token, "IMPULSE") )
	    token_type = UF_IMPULSE;
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
	  else if ( 0 == strcasecmp(token, "TO") )
	    token_type = UF_TO;
	  else if ( 0 == strcasecmp(token, "SWAP") )
	    token_type = UF_SWAP;
	  else if ( 0 == strcasecmp(token, "DROP") )
	    token_type = UF_DROP;
	  else if ( 0 == strcasecmp(token, "IF") )
	    token_type = UF_IF;
	  else if ( 0 == strcasecmp(token, "ELSE") )
	    token_type = UF_ELSE;
	  else if ( 0 == strcasecmp(token, "THEN") )
	    token_type = UF_THEN;
	  else if ( 0 == strcasecmp(token, "BEGIN") )
	    token_type = UF_BEGIN;
	  else if ( 0 == strcasecmp(token, "WHILE") )
	    token_type = UF_WHILE;
	  else if ( 0 == strcasecmp(token, "REPEAT") ||
		    0 == strcasecmp(token, "CONTINUE") )
	    token_type = UF_REPEAT;
	  else if ( 0 == strcasecmp(token, "UNTIL") )
	    token_type = UF_UNTIL;
	  else if ( 0 == strcasecmp(token, "AGAIN") )
	    token_type = UF_AGAIN;
	  else if ( 0 == strcasecmp(token, "LEAVE") ||
		    0 == strcasecmp(token, "BREAK") )
	    token_type = UF_LEAVE;
	  else if ( 0 == strcasecmp(token, "DEPTH") )
	    token_type = UF_DEPTH;
	  else if ( 0 == strcmp(token, "<") ||
		    0 == strcasecmp(token, "_LT"))
	    token_type = UF_LT;
	  else if ( 0 == strcmp(token, "<=") ||
		    0 == strcasecmp(token, "_LE"))
	    token_type = UF_LE;
	  else if ( 0 == strcmp(token, "=") ||
		    0 == strcmp(token, "==") ||
		    0 == strcasecmp(token, "_EQ"))
	    token_type = UF_EQ;
	  else if ( 0 == strcmp(token, "<>") ||
		    0 == strcmp(token, "!=") ||
		    0 == strcasecmp(token, "_NE"))
	    token_type = UF_NE;
	  else if ( 0 == strcmp(token, ">=") ||
		    0 == strcasecmp(token, "_GE"))
	    token_type = UF_GE;
	  else if ( 0 == strcmp(token, ">") ||
		    0 == strcasecmp(token, "_GT"))
	    token_type = UF_GT;
	  else
	    {
	      char *endptr;
	      r = dipole_parse_magnitude_ext(token, &endptr);
	      if ( NULL == endptr || *endptr == '\0' )
		{
		  token_type = UF_VALUE_REAL;
		}
	      else
		{
		  token_type = UF_VALUE_SIMULATION;
		  r = 0.;
		}
	    }
	  uforth_token_t *new_token = uforth_token_new(token_type, token, r);
	  *uf_ctx->last = new_token;
	  uf_ctx->last = &new_token->next;
	}
    }
  free(code_str);
  *uf_ctxp = uf_ctx;
  return SUCCESS;
}

status_t
uforth_compile(const char *buf, int stack_size,
	       uforth_context_t **uf_ctxp)
{
  return compile(buf, stack_size, true, uf_ctxp);
}

status_t
uforth_compile_command(const char *buf, int stack_size,
		      uforth_context_t **uf_ctxp)
{
  return compile(buf, stack_size, false, uf_ctxp);
}

static status_t
pop_dB(uforth_context_t *uf_ctx, yana_complex_t *cp)
{
  int pos = uf_ctx->stack_pos-1;
  if ( uf_ctx->stack_pos <= 0 )
    {
      ERROR("stack underflow");
      return FAILURE;
    }

  switch (uf_ctx->stack[pos].type)
    {
    case UF_VALUE_DB:
      *cp = uf_ctx->stack[pos].v;
      break;
    default:
      ERROR("Unexpected type in stack, dB was expected");
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
      ERROR("stack underflow");
      return FAILURE;
    }

  switch (uf_ctx->stack[pos].type)
    {
    case UF_VALUE_REAL:
    case UF_VALUE_COMPLEX:
      *cp = uf_ctx->stack[pos].v;
      break;
    case UF_VALUE_DB:
      ERROR("Unexpected TOS dB value, dB is a printable only data type");
      return FAILURE;
    default:
      ERROR("Unexpected type in stack, complex was expected");
      return FAILURE;
    }
  uf_ctx->stack_pos = pos;
  return SUCCESS;
}

static status_t
pop_real(uforth_context_t *uf_ctx, yana_real_t *rp)
{
  yana_complex_t c;
  status_t status = pop_complex(uf_ctx, &c);
  if (SUCCESS != status) {
    return status;
  }
  if (cimag(c) != 0) {
    ERROR("stack element is complex, real was expected");
    return FAILURE;
  }
  *rp = creal(c);
  return SUCCESS;
}

static status_t
push_real(uforth_context_t *uf_ctx, yana_real_t r)
{
  int pos = uf_ctx->stack_pos;
  if ( pos+1 >= uf_ctx->stack_size )
    {
      ERROR("stack overflow");
      return FAILURE;
    }
  uf_ctx->stack[pos].type = UF_VALUE_REAL;
  uf_ctx->stack[pos].v = r;
  uf_ctx->stack_pos = pos + 1;
  return SUCCESS;
}

static status_t
push_complex(uforth_context_t *uf_ctx, yana_complex_t c)
{
  int pos = uf_ctx->stack_pos;
  if ( pos+1 >= uf_ctx->stack_size )
    {
      ERROR("stack overflow");
      return FAILURE;
    }
  uf_ctx->stack[pos].type = UF_VALUE_COMPLEX;
  uf_ctx->stack[pos].v = c;
  uf_ctx->stack_pos = pos + 1;
  return SUCCESS;
}

static status_t
push_dB(uforth_context_t *uf_ctx, yana_complex_t dB)
{
  int pos = uf_ctx->stack_pos;
  if ( pos+1 >= uf_ctx->stack_size )
    {
      ERROR("stack overflow");
      return FAILURE;
    }
  uf_ctx->stack[pos].type = UF_VALUE_DB;
  uf_ctx->stack[pos].v = dB;
  uf_ctx->stack_pos = pos + 1;
  return SUCCESS;
}

#define POP_DB(_op_, _var_)						\
  do {									\
    status = pop_dB(uf_ctx, &(_var_));                                  \
    if ( SUCCESS != status )						\
      {									\
	ERROR(_op_" expects a dB value");				\
	goto loop_exit;							\
      }									\
  } while (0)
#define POP_REAL(_op_, _var_)						\
  do {									\
    status = pop_real(uf_ctx, &(_var_));				\
    if ( SUCCESS != status )						\
      {									\
	ERROR(_op_" expects a real value");				\
	goto loop_exit;							\
      }									\
  } while (0)
#define POP_COMPLEX(_op_, _var_)					\
  do {									\
    status = pop_complex(uf_ctx, &(_var_));				\
    if ( SUCCESS != status )						\
      {									\
	ERROR(_op_" expects a complex value");				\
	goto loop_exit;							\
      }									\
  } while (0)

#define PUSH_DB(_op_, _val_)						\
  do {									\
    status = push_dB(uf_ctx, (_val_));                                  \
    if ( SUCCESS != status )						\
      {									\
	ERROR("on "_op_);						\
	goto loop_exit;							\
      }									\
  } while (0)
#define PUSH_REAL(_op_, _val_)						\
  do {									\
    status = push_real(uf_ctx, (_val_));				\
    if ( SUCCESS != status )						\
      {									\
	ERROR("on "_op_);						\
	goto loop_exit;							\
      }									\
  } while (0)

#define PUSH_COMPLEX(_op_, _val_)					\
  do {									\
    status = push_complex(uf_ctx, (_val_));				\
    if ( SUCCESS != status )						\
      {									\
	ERROR("on "_op_);						\
	goto loop_exit;							\
      }									\
  } while (0)

#define HEAD_TYPE(_var_)				\
  do {							\
    if ( uf_ctx->stack_pos < 1 )			\
      {							\
	status = FAILURE;				\
	ERROR("stack underflow");			\
	goto loop_exit;					\
      }							\
    (_var_)=uf_ctx->stack[uf_ctx->stack_pos-1].type;	\
  } while (0)

#define L_PUSH(tok)						\
  do {								\
    if ( l_stack_pos > sizeof(l_stack)/sizeof(*l_stack) )	\
      {								\
	ERROR("loop stack overflow");				\
	status = FAILURE;					\
	goto loop_exit;						\
      }								\
    l_stack[l_stack_pos++]=tok;					\
  } while (0)
#define L_POP(tok)						\
  do {								\
    if ( l_stack_pos < 1 )					\
      {								\
	ERROR("loop stack underflow");				\
	status = FAILURE;					\
	goto loop_exit;						\
      }								\
    tok=l_stack[--l_stack_pos];					\
  } while (0)

#define L_DROP()						\
  do {								\
    if ( l_stack_pos < 1 )					\
      {								\
	ERROR("loop stack underflow");				\
	status = FAILURE;					\
	goto loop_exit;						\
      }								\
    --l_stack_pos;						\
  } while (0)

#define L_HEAD(tok)						\
  do {								\
    if ( l_stack_pos == 0 )					\
      {								\
	ERROR("loop stack empty");				\
	status = FAILURE;					\
	goto loop_exit;						\
      }								\
    tok = l_stack[l_stack_pos-1];				\
  } while (0)


static yana_real_t
get_freq(simulation_context_t *sc, int i) {
  yana_real_t freq = 1;
  if (sc) {
    freq = simulation_context_get_f(sc, i);
    if (sc->impulse) {
      //freq = freq / (double)sc->log_hz_max;
    }
  }
    return freq;
}
static yana_real_t
get_actual_freq(simulation_context_t *sc, int i) {
  return get_freq(sc, i);
  //return sc?simulation_context_get_f(sc, i):1.L
}

static int counter = 0;
double start;


void ctor(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  start = 1000000000 * ts.tv_sec + ts.tv_nsec;
}

__attribute__((destructor))
static void dtor(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  double end = 1000000000 * ts.tv_sec + ts.tv_nsec;
  fprintf(stderr,
          "%d INSTRUCTIONS\n"
          "T = %fms\n"
          "%f MIPS\n"
          "cycle %fns\n"
          , counter
          , (end-start) * 1e-6
          , (double)counter / ((end-start) * 1e-3)
          , (end-start)/counter
          );
}

static status_t
uforth_execute_step(uforth_context_t *uf_ctx,
		    simulation_context_t *sc,
		    simulation_t *simulation,
		    uforth_heap_t *heap,
		    yana_complex_t *resultp,
		    int i,
                    int smoothing,
                    int impulse,
                    uforth_output_t *output)
{
  uforth_token_t *l_stack[16];
  int l_stack_pos = 0;
  status_t status = SUCCESS;
  uforth_token_t *token;
  yana_real_t r1, r2, r3;
  yana_complex_t c1, c2;
  yana_real_t freq = get_freq(sc, i);
  yana_real_t actual_freq = get_actual_freq(sc, i);
  int s = sc?simulation_context_get_n_samples(sc):0;
  uforth_token_type_t head_type;
  bool printed = false;
  bool end = false;
  bool free_heap = false;
  if ( NULL == heap )
    {
      free_heap = true;
      heap = uforth_heap_new();
    }
  for ( token = uf_ctx->first ;
	token && !end;
	token = token->next )
    {
      ++counter;
      switch (token->type)
	{
	case UF_FREEAIR:
	  POP_REAL("( x X -- x ) FREEAIR", r2);
	  POP_REAL("( X x -- x ) FREEAIR", r1);
	  PUSH_COMPLEX("FREEAIR", free_air_impedance(actual_freq, r1, r2));
	  break;
	case UF_DIRIMP:
	  POP_REAL("( x x X -- x ) FREEAIR", r3); // theta
	  POP_REAL("( x X x -- x ) FREEAIR", r2); // r
	  POP_REAL("( X x x -- x ) FREEAIR", r1); // Sd
          PUSH_COMPLEX("DIRIMP", free_air_dir_impedance(actual_freq, r2, r1, r3));
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
	case UF_SQRT:
	  POP_COMPLEX("( X -- x ) SQRT", c1);
	  PUSH_COMPLEX("SQRT", csqrt(c1));
	  break;
	case UF_LN:
	  POP_COMPLEX("( X -- x ) LN", c1);
	  PUSH_COMPLEX("LN", clog(c1));
	  break;
	case UF_COS:
	  POP_COMPLEX("( X -- x ) COS", c1);
	  PUSH_COMPLEX("COS", ccos(c1));
	  break;
	case UF_SIN:
	  POP_COMPLEX("( X -- x ) SIN", c1);
	  PUSH_COMPLEX("SIN", csin(c1));
	  break;
	case UF_TAN:
	  POP_COMPLEX("( X -- x ) TAN", c1);
	  PUSH_COMPLEX("TAN", ctan(c1));
	  break;
	case UF_ACOS:
	  POP_COMPLEX("( X -- x ) ACOS", c1);
	  PUSH_COMPLEX("ACOS", cacos(c1));
	  break;
	case UF_ASIN:
	  POP_COMPLEX("( X -- x ) ASIN", c1);
	  PUSH_COMPLEX("ASIN", casin(c1));
	  break;
	case UF_ATAN:
	  POP_COMPLEX("( X -- x ) ATAN", c1);
	  PUSH_COMPLEX("ATAN", catan(c1));
	  break;
	case UF_LOG:
	  POP_COMPLEX("( X -- x ) LOG", c1);
	  PUSH_COMPLEX("LOG", clog10(c1));
	  break;
	case UF_PAR:
	  POP_COMPLEX("( x X -- x ) PAR", c2);
	  POP_COMPLEX("( X x -- x ) PAR", c1);
	  PUSH_COMPLEX("PAR", (c1*c2)/(c1+c2) );
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
	case UF_ANGLE:
	  POP_REAL("( x X -- x ) ANGLE", r2);
	  POP_REAL("( X x -- x ) ANGLE", r1);
	  if ( fabs(r1-r2-2.*M_PI) > fabs(r1-r2) )
	    {
	      if ( fabs(r1-r2+2.*M_PI) > fabs(r1-r2) )
		PUSH_REAL("ANGLE", r1-r2);
	      else
		PUSH_REAL("ANGLE", r1-r2+2.*M_PI);
	    }
	  else
	    {
	      if ( fabs(r1-r2+2.*M_PI) > fabs(r1-r2-2.*M_PI) )
		PUSH_REAL("ANGLE", r1-r2-2.*M_PI);
	      else
		PUSH_REAL("ANGLE", r1-r2+2.*M_PI);
	    }
	  break;
	case UF_PDELAY:
	  POP_REAL("( X -- x ) PDELAY", r1);
	  PUSH_REAL("PDELAY",  - r1 /( 2. * M_PI * actual_freq ) );
	  break;
	case UF_PREV_STEP:
	  if ( 0 == i )
	    {
	      end = true;
	      break;
	    }
	  --i;
	  freq = get_freq(sc, i);
          actual_freq = get_actual_freq(sc, i);
	  break;
	case UF_NEXT_STEP:
	  if ( s-1 == i )
	    {
	      end = true;
	      break;
	    }
	  ++i;
	  freq = get_freq(sc, i);
          actual_freq = get_actual_freq(sc, i);
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
	case UF_RHO:
	  PUSH_REAL("RHO", YANA_RHO);
	  break;
	case UF_C:
	  PUSH_REAL("C", YANA_C);
	  break;
	case UF_MU:
	  PUSH_REAL("MU", YANA_MU);
	  break;
        case UF_IMPULSE:
          PUSH_REAL("IMPULSE", (yana_real_t)impulse);
          break;
	case UF_F:
          PUSH_REAL("F", freq);
	  break;
	case UF_S:
	  PUSH_REAL("S", i);
	  break;
	case UF_I:
	  PUSH_COMPLEX("I", 0. + I * 1.);
	  break;
	case UF_DB:
	  POP_COMPLEX("( X -- x ) DB", c1);
	  PUSH_DB("DB", c1);
	  //PUSH_REAL("DB", 20. * log10(cabs(c1)));
	  break;
	case UF_DBSPL:
	  POP_COMPLEX("( X -- x ) DBSPL", c1);
          PUSH_DB("DBSPL", c1/20e-6);
	  //PUSH_REAL("DBSPL", 20. * log10(cabs(c1)/20e-6));
	  break;
	case UF_DOT:
	  HEAD_TYPE(head_type);
	  if ( UF_VALUE_REAL == head_type )
	    {
	      POP_REAL("( X -- ) DOT", r1);
              uforth_output_dot(output, r1);
	      //fprintf(stdout, "%1.12g\t", (double)r1);
	    }
	  else if ( UF_VALUE_COMPLEX == head_type )
	    {
	      POP_COMPLEX("( X -- ) DOT", c1);
              uforth_output_dot(output, c1);
	      //fprintf(stdout, "%1.12g\t", (double)cabs(c1));
	    }
	  else if ( UF_VALUE_DB == head_type )
	    {
	      POP_DB("( X -- ) DOT", c1);
              uforth_output_dot_dB(output, c1);
	      //fprintf(stdout, "%1.12g\t", (double)cabs(c1));
	    }
	  printed=true;
	  break;
	case UF_DUP:
	  POP_COMPLEX("( X -- x x ) DUP", c1);
	  PUSH_COMPLEX("DUP", c1);
	  PUSH_COMPLEX("DUP", c1);
	  break;
	case UF_TO:
	  token=token->next;
	  if ( NULL == token )
	    {
	      ERROR("TO: end of instructions stream");
	      status = FAILURE;
	      goto loop_exit;
	    }
	  if ( token->type != UF_VALUE_SIMULATION )
	    {
	      ERROR("TO: %s is not a valid word to be set", token->symbol);
	      status = FAILURE;
	      goto loop_exit;
	    }
          {
            HEAD_TYPE(head_type);
            if (head_type == UF_VALUE_DB) {
              POP_DB("(X -- ) TO", c1);
            } else {
              POP_COMPLEX("(X -- ) TO", c1);
            }
            uforth_heap_set(heap, head_type, token->symbol, c1);
          }
	  break;
	case UF_SWAP:
	  if ( uf_ctx->stack_pos < 2 )
	    {
	      ERROR("SWAP: Stack underflow");
	      status = FAILURE;
	      goto loop_exit;
	    }
	  token_swap(&uf_ctx->stack[uf_ctx->stack_pos-1],
		     &uf_ctx->stack[uf_ctx->stack_pos-2]);
	  break;
	case UF_DROP:
	  POP_COMPLEX("(X -- ) DROP", c1);
	  break;
	case UF_IF:
	  POP_COMPLEX("(X -- ) IF", c1);
	  if ( 0. == cabs(c1) )
	    {
	      int depth=-1;
	      uforth_token_t *orig_position = token;
	      while ( ( token->type != UF_ELSE && token->type != UF_THEN ) || depth != 0 )
		{
		  if ( token->type == UF_IF ) ++depth;
		  if ( token->type == UF_THEN ) --depth;
		  token = token->next;
		  if ( NULL == token )
		    {
		      ERROR("IF: no matching ELSE or THEN found");
		      token = orig_position;
		      status = FAILURE;
		      goto loop_exit;
		    }
		}
	    }
	  break;
	case UF_ELSE:
	  {
	    int depth=0;
	    uforth_token_t *orig_position = token;
	    while ( token->type != UF_THEN || depth != 0 )
	      {
		if ( token->type == UF_IF ) ++depth;
		if ( token->type == UF_THEN ) --depth;
		token = token->next;
		if ( NULL == token )
		  {
		    ERROR("ELSE: no matching THEN found");
		    token = orig_position;
		    status = FAILURE;
		    goto loop_exit;
		  }
	      }
	  }
	  break;
	case UF_THEN:
	  //noop
	  break;
	case UF_BEGIN:
	  L_PUSH(token);
	  break;
	case UF_WHILE:
	  POP_COMPLEX("(X -- ) WHILE", c1);
	  if ( 0. == c1 )
	    {
	      L_DROP();
	      int depth=0;
	      uforth_token_t *orig_position = token;
	      while ( token->type != UF_REPEAT || 0 != depth)
		{
		  if ( token->type == UF_BEGIN ) ++depth;
		  if ( token->type == UF_UNTIL ) --depth;
		  if ( token->type == UF_REPEAT ) --depth;
		  if ( token->type == UF_AGAIN ) --depth;
		  token=token->next;
		  if ( NULL == token )
		    {
		      ERROR("WHILE: no matching REPEAT found");
		      token = orig_position;
		      status = FAILURE;
		      goto loop_exit;
		    }
		}
	    }
	  break;
	case UF_REPEAT:
	  L_HEAD(token);
	  break;
	case UF_UNTIL:
	  POP_COMPLEX("(X -- ) UNTIL", c1);
	  if ( 0. == c1 )
	    L_HEAD(token);
	  else
	    L_DROP();
	  break;
	case UF_AGAIN:
	    L_HEAD(token);
	  break;
	case UF_LEAVE:
	  {
	    int depth = 0;
	    L_DROP();
	    uforth_token_t *orig_position = token;
	    while ( ! ( ( token->type == UF_UNTIL ||
			  token->type == UF_REPEAT ||
			  token->type == UF_AGAIN ) && depth == 0 ) )
	      {
		if ( token->type == UF_BEGIN ) ++depth;
		if ( token->type == UF_UNTIL ) --depth;
		if ( token->type == UF_REPEAT ) --depth;
		if ( token->type == UF_AGAIN ) --depth;
		token = token->next;
		if ( NULL == token )
		  {
		    ERROR("LEAVE: no matching UNTIL|REPEAT|AGAIN found");
		    token = orig_position;
		    status = FAILURE;
		    goto loop_exit;
		  }
	      }
	  }
	  break;
	case UF_DEPTH:
	  PUSH_REAL("DEPTH", uf_ctx->stack_pos);
	  break;
	case UF_LT:
	case UF_LE:
	case UF_EQ:
	case UF_NE:
	case UF_GE:
	case UF_GT:
	  POP_COMPLEX("(x X -- ) IF", c2);
	  POP_COMPLEX("(X x -- ) IF", c1);
	  r1=cabs(c1); r2=cabs(c2);
	  PUSH_REAL("comparison",
		    UF_LT == token->type ? ( r1<r2)
		    : UF_LE == token->type ? (r1<=r2)
		    : UF_EQ == token->type ? (r1==r2)
		    : UF_NE == token->type ? (r1!=r2)
		    : UF_GE == token->type ? (r1>=r2)
		    : UF_GT == token->type ? (r1>r2)
		    : 0);
	  break;
	case UF_VALUE_REAL:
	  PUSH_REAL("real literal", token->v);
	  break;
	case UF_VALUE_DB:
	  assert(!"not possible");
	  PUSH_DB("dB literal", token->v);
	  break;
	case UF_VALUE_COMPLEX:
	  assert(!"not possible");
	  PUSH_COMPLEX("complex literal", token->v);
	  break;
	case UF_VALUE_SIMULATION:
	  {
	    yana_complex_t *sim_array;
	    const uforth_token_t *heap_token = heap_token = uforth_heap_get(heap, token->symbol);
	    if ( NULL != heap_token )
	      {
		PUSH_COMPLEX("heap word", heap_token->v);
	      }
	    else
	      {
		if ( token->symbol[0] != 'v'  &&  token->symbol[0] != 'I' )
		  {
		    ERROR("Unknown symbol '%s'\n", token->symbol);
		    if ( sc )
		      HINT("dipoles start with 'I' and nodes start with 'v'");
		    status = FAILURE;
		    goto loop_exit;
		  }
		sim_array = simulation_result(simulation, token->symbol+1);
		if ( NULL == sim_array )
		  {
		    ERROR("Unknown symbol '%s'", token->symbol);
		    status = FAILURE;
		    goto loop_exit;
		  }
                yana_complex_t value = sim_array[i];
                if (smoothing) {
                  if (0) {
                    int j;
                    int b = i - smoothing;
                    if (b < 0) b = 0;
                    for (j = b; j < i ; ++j) {
                      value += sim_array[j];
                    }
                    value /= (i - b);
                  } else {
                    int j;
                    int b = i + smoothing;
                    if (b >= simulation_context_get_n_samples(sc)) b = simulation_context_get_n_samples(sc);
                    int n = 1;
                    for (j = i + 1; j <= b ; ++j) {
                      ++n;
                      //TODO: invalid read here
                      value += sim_array[j];
                    }
                    value /= n;
                  }
                }
		PUSH_COMPLEX("sim", value);
	      }
	  }
	  break;
	}
    }
 loop_exit:

  if (printed) {
    //fprintf(stdout, "\n");
  }

  if ( SUCCESS != status && NULL != token )
    {
      fputs("ERROR: is here: ", stderr);
      uforth_token_t *t;
      for ( t = uf_ctx->first ;
	    t != NULL ;
	    t = t->next )
	{
	  if ( t == token )
	    {
	      fprintf(stderr, ">>>%s<<<", t->symbol);
	      break;
	    }
	  else
	    {
	      fprintf(stderr, "%s ", t->symbol);
	    }
	}
      fputs("\n", stderr);
    }
  else if ( l_stack_pos != 0 )
    {
      ERROR("loop stack not empty at the end of the processing");
      status = FAILURE;
    }
  else if ( uf_ctx->stack_pos != 0 && NULL == resultp )
    {
      if ( !end )
	WARNING("stack not empty at the end of the processing");
      uf_ctx->stack_pos = 0;
    }

  if ( SUCCESS == status && NULL != resultp)
    {
      if ( uf_ctx->stack_pos != 1 )
	{
	  ERROR("one result was expected and stack size is %d",
		  uf_ctx->stack_pos);
	  status = FAILURE;
	}
      else
	POP_COMPLEX("RESULT", *resultp);
    }
  if ( free_heap )
    uforth_heap_free(heap);

  return status;
}

status_t
uforth_execute(uforth_context_t *uf_ctx,
	       simulation_context_t *sc,
	       simulation_t *simulation,
	       uforth_heap_t *heap,
	       yana_complex_t *resultp,
               int smoothing,
               int impulse,
               uforth_output_t **outputp)
{
  ctor();
  int i, s;
  status_t status;
  uforth_output_t *output = uforth_output_new();
  if ( NULL == sc || NULL == simulation )
    s = 1;
  else
    s = simulation_context_get_n_samples(sc);
  for ( i = 0 ; i < s ; ++i )
    {
      status = uforth_execute_step(uf_ctx, sc, simulation, heap, resultp, i,
                                   smoothing, impulse, output);
      if ( SUCCESS != status )
	{
	  ERROR("Execution failed");
	  return status;
	}
      uforth_output_newline(output);
    }
  if (outputp) {
    *outputp = output;
  } else {
    uforth_output_free(output);
  }
  return SUCCESS;
}
