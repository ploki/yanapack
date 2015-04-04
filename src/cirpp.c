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

#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <uforth.h>


struct subckt
{
  char *name;
  char **input;
  int n_inputs;
  char *netlist;
  size_t netlist_size;
  FILE *saved_stream;
};

typedef struct subckt subckt_t;

struct subckt_heap
{
  yana_map_t *map;
};

typedef struct subckt_heap subckt_heap_t;

void subckt_free(subckt_t *sub)
{
  if ( sub )
    {
      int i;
      for ( i = 0 ; i < sub->n_inputs ; ++i )
	{
	  free(sub->input[i]);
	}
      free(sub->input);
      free(sub->name);
      free(sub->netlist);
    }
  free(sub);
}


static void subckt_entry_free(yana_pair_t *pair)
{
  subckt_free(pair->value);
}
static int subckt_entry_compar(const yana_pair_t *lhs,
			       const yana_pair_t *rhs)
{
  return strcmp((const char*)lhs->key, (const char*)rhs->key);
}

static subckt_heap_t *
subckt_heap_new(void)
{
  subckt_heap_t *heap = calloc(1, sizeof *heap);
  if ( NULL == heap )
    return NULL;
  heap->map = yana_map_new(subckt_entry_compar, subckt_entry_free);
  if ( NULL == heap->map )
    {
      free(heap);
      return NULL;
    }
  return heap;
}

static void
subckt_heap_free(subckt_heap_t *subckt_heap)
{
  yana_map_free(subckt_heap->map);
  free(subckt_heap);
}

static subckt_t *
subckt_begin(const char *name, FILE **current_stream)
{
  subckt_t *sub = calloc(1, sizeof *sub);
  if ( NULL == sub )
    return NULL;
  sub->name = strdup(name);
  sub->saved_stream = *current_stream;
  *current_stream = open_memstream(&sub->netlist, &sub->netlist_size);
  return sub;
}

static void
subckt_push_node(subckt_t *sub, const char *node)
{
  char **tmp;
  sub->n_inputs++;
  tmp = realloc(sub->input, sizeof(*sub->input)*sub->n_inputs);
  assert( NULL != tmp );
  sub->input=tmp;
  sub->input[sub->n_inputs-1] = strdup(node);
}
static void
subckt_ends(subckt_heap_t *subckt_heap, subckt_t *sub, FILE **output)
{
  fclose(*output);
  *output = sub->saved_stream;
  yana_pair_t *pair = yana_pair_new(sub->name, sub);
  assert( NULL != pair );
  yana_map_set(subckt_heap->map, pair);
}

static const subckt_t *
subckt_get(subckt_heap_t *subckt_heap, const char *name)
{
  const yana_pair_t *pair = yana_map_get(subckt_heap->map, name);
  if ( NULL == pair )
    return NULL;
  return pair->value;
}

static status_t
cc_expand(subckt_heap_t *subckt_heap, const char *line_const, FILE *output)
{
  status_t status = FAILURE;
  uforth_heap_t *uf_heap = uforth_heap_new();
  char *line = strdup(line_const);
  const subckt_t *sub;
  char *tmp = NULL, *tmp2 = NULL;

  char **token = NULL;
  char n_tokens = 0;
  char *name = strtok_r(line, " ", &tmp);
  char *word;
  bool param_found=false;
  char *subckt_name;
  assert( NULL != name );
  name = strdup(name);
  while ( ( word = strtok_r(NULL, " ", &tmp) ) )
    {
      if ( 0 == strcasecmp(word, "param:") )
	{
	  param_found=true;
	  if ( 0 == n_tokens )
	    {
	      fprintf(stderr, "ERROR: syntax on %s\n", name);
	      goto end;
	    }
	  subckt_name = token[n_tokens-1];
	  break;
	}
      ++n_tokens;
      token=realloc(token, sizeof(*token)*n_tokens);
      assert( NULL != token );
      token[n_tokens-1]=strdup(word);
    }
  if ( 0 == n_tokens )
    {
      fprintf(stderr, "ERROR: syntax on %s\n", name);
      goto end;
    }
  if ( !param_found )
    {
      subckt_name = token[n_tokens-1];
    }
  
  if ( NULL == subckt_name )
    {
      fprintf(stderr, "ERROR: unable to get subckt name for %s\n", name);
      goto end;
    }
  sub = subckt_get(subckt_heap, subckt_name);
  if ( NULL == sub )
    {
      fprintf(stderr, "ERROR: unknown subckt: %s for %s\n", subckt_name, name);
      goto end;
    }

  //parse param line
  const char *o, *c=strstr(line_const, token[n_tokens-1])+strlen(token[n_tokens-1]);
  for (;;)
    {
      status = FAILURE;
      o = strchr(c, '{');
      if ( NULL == o )
	break;
      c = strchr(o+1, '}');
      if ( NULL == c )
	{
	  fprintf(stderr, "ERROR: missing closing brace in %s parameters\n", name);
	  goto end;
	}
      char *buf = calloc(1, c-o);
      memcpy(buf, o+1, c-o-1);
      uforth_context_t *uf_ctx = NULL;
      status = uforth_compile_command(buf, 1000, &uf_ctx);
      if ( SUCCESS != status )
	  goto end;
      status = uforth_execute(uf_ctx, NULL, NULL, uf_heap, NULL);
      if ( SUCCESS != status )
	goto end;
      uforth_free(uf_ctx);
    }
  
  //execute forth in netlist
  char *local_buf=NULL;
  size_t local_size;
  FILE *local = open_memstream(&local_buf, &local_size);
  
  const char *p=sub->netlist;
  for (;;)
    {
      o = strchr(p, '{');
      if ( NULL == o )
	{
	  fprintf(local, "%s\n",p);
	  break;
	}
      else
	{
	  fwrite(p, 1, o-p-1, local);
	}
      c = strchr(o+1, '}');
      if ( NULL == c )
	{
	  fprintf(stderr, "ERROR: missing closing brace in  %s subckt netlist\n", sub->name);
	  goto end;
	}
      p=c+1;
      char *buf = calloc(1, c-o);
      memcpy(buf, o+1, c-o-1);
      uforth_context_t *uf_ctx = NULL;
      status = uforth_compile_command(buf, 1000, &uf_ctx);
      if ( SUCCESS != status )
	  goto end;
      yana_complex_t result;
      status = uforth_execute(uf_ctx, NULL, NULL, uf_heap, &result);
      if ( SUCCESS != status )
	goto end;
      fprintf(local, " %1.12g ", cabs(result));
      uforth_free(uf_ctx);
    }
  fclose(local);
  free(line);
  //replace nodes and add postfix in netlist
  for( line = strtok_r(local_buf, "\n", &tmp) ;
       line != NULL ;
       line = strtok_r(NULL, "\n", &tmp) )
    {
      int word_number=0;
      for ( word = strtok_r(line, " ", &tmp2) ;
	    word != NULL ;
	    word = strtok_r(NULL, " ", &tmp2 ) )
	{
	  if (0==word_number)
	    {
	      fprintf(output, "%s:%s ", word, name);
	    }
	  else
	    {
	      int i;
	      for ( i = 0 ; i < sub->n_inputs ; ++i )
		{
		  if ( 0 == strcmp(word, sub->input[i]) )
		    {
		      fprintf(output, "%s ", token[i]);
		      break;
		    }
		}
	      if ( i == sub->n_inputs )
		{
		  if ( word_number < 3 )
		    fprintf(output, "%s:%s ", word, name);
		  else
		    fprintf(output, "%s ", word);
		}
	    }
	  ++word_number;
	}
      fprintf(output, "\n");
    }
  
  free(local_buf);
  
  status = SUCCESS;
 end:
  free(line);
  uforth_heap_free(uf_heap);
  return status;
}

static status_t
cirpp_internal(const char *input_const, FILE *output)
{
  char *input = strdup(input_const);
  char *p;
  status_t status = FAILURE;
  subckt_t *sub = NULL;
  subckt_heap_t *subckt_heap = subckt_heap_new();
  
  if ( NULL == output )
    goto end;
  for ( p = input ;
	*p != '\0' ;
	++p )
    {
      if ( p[0] == '\\' )
	{
	  p[0] = ' ';
	  if ( p[1] == '\\' )
	    {
	      ++p;
	    }
	  else if ( p[1] == '\n' )
	    {
	      p[1] = ' ';
	      ++p;
	    }
	  else if ( p[1] == '\r' && p[2] == '\n' )
	    {
	      p[1] = ' '; p[2] = ' ';
	      p+=2;
	    }
	}
      else if ( p[0] == '\t' )
	p[0] = ' ';
    }
  char *tmp1 = NULL, *tmp2 = NULL, *line, *saved_line;
  for ( line = strtok_r(input, "\n", &tmp1) ;
	line != NULL ;
	line = strtok_r(NULL, "\n", &tmp1) )
    {
      saved_line = strdup(line);
      if ( '*' == line[0] ||
	   '$' == line[0] ||
	   '!' == line[0] )
	{
	  fprintf(output, "%s\n", line);
	  continue;
	}

      char *first_word = strtok_r(saved_line, " ", &tmp2);

      if ( NULL == first_word )
	{
	  fprintf(output, "\n");
	}
      else if ( 0 == strcasecmp(first_word, ".include") )
	{
	}
      else if ( 0 == strcasecmp(first_word, ".subckt") )
	{
	  if ( NULL != sub )
	    {
	      fprintf(stderr, "ERROR: nested .subckt is forbidden\n");
	      status = FAILURE;
	      goto end;
	    }
	  char *sub_ckt_name = strtok_r(NULL, " ", &tmp2);
	  sub = subckt_begin(sub_ckt_name, &output);
	  char *node;
	  while ( (node = strtok_r(NULL, " ", &tmp2)) )
	    {
	      subckt_push_node(sub, node);
	    }
	}
      else if ( 0 == strcasecmp(first_word, ".param") )
	{
	  //skip this line
	}
      else if ( 0 == strcasecmp(first_word, ".ends") )
	{
	  if ( NULL == sub )
	    {
	      fprintf(stderr, "ERROR: .ends outside a .subckt is forbidden\n");
	      status = FAILURE;
	      goto end;
	    }
	  subckt_ends(subckt_heap, sub, &output);
	  sub = NULL;
	}
      else if ( first_word[0] == 'x' || first_word[0] == 'X' )
	{
	  status = cc_expand(subckt_heap, line, output);
	  if ( SUCCESS != status )
	    {
	      goto end;
	    }
	}
      else
	fprintf(output, "%s\n", line);
    }
  
  subckt_heap_free(subckt_heap);
  status = SUCCESS;
 end:
  return status;
}


static status_t
cirpp(const char *input_const, char **outputp)
{
  size_t size;
  status_t status;
  FILE *output;
  output = open_memstream(outputp, &size);
  status = cirpp_internal(input_const, output);
  fclose(output);
  return status;
}


static
status_t
cirpp_load_internal(const char *netlist_file, int depth, FILE *output)
{
  status_t status = FAILURE;
  struct stat st;
  int ret;
  char *netlist_buf, *tmp = NULL, *tmp2 = NULL, *line, *to_include;

  if ( depth > 10 )
    {
      fprintf(stderr, "ERROR: too many recursions in .include\n");
      return FAILURE;
    }
  
  ret = stat(netlist_file, &st);
  if (ret < 0 )
    {
      perror(netlist_file);
      exit(EXIT_FAILURE);
    }
  netlist_buf = malloc(st.st_size+1);
  if ( NULL == netlist_buf )
    {
      fprintf(stderr, "ERROR: ENOMEM\n");
      exit(EXIT_FAILURE);
    }
  FILE *f = fopen(netlist_file, "r");
  if ( NULL == f )
    {
      perror(netlist_file);
      exit(EXIT_FAILURE);
    }
  size_t bytes = fread(netlist_buf, 1, st.st_size, f);
  assert( bytes == st.st_size );
  netlist_buf[st.st_size]='\0';

  for ( line = strtok_r(netlist_buf, "\r\n", &tmp) ;
	line != NULL ;
	line = strtok_r(NULL, "\r\n", &tmp) )
    {
      if ( 0 ==  strncasecmp(line, ".include", sizeof(".include")-1) )
	{
	  line+=sizeof(".include")-1;
	  if ( line[0] != ' ' && line[0] != '\t' )
	    {
	      fprintf(stderr, "ERROR: .include syntax error\n");
	      status = FAILURE;
	      goto end;
	    }
	  to_include = strtok_r(line, " \t", &tmp2);
	  if ( NULL == to_include )
	    {
	      fprintf(stderr, "ERROR: .include syntax error: no file specified\n");
	      status = FAILURE;
	      goto end;
	    }
	  status = cirpp_load_internal(to_include, depth+1, output);
	  if ( SUCCESS != status )
	    goto end;
	}
      else
	{
	  fprintf(output, "%s\n", line);
	}
    }
  status = SUCCESS;
 end:
  free(netlist_buf);
  fclose(f);
  return status;
}

status_t
cirpp_load(const char *netlist_file, char **outputp)
{
  size_t size;
  status_t status;
  FILE *output;
  char *tmpbuf;
  output = open_memstream(&tmpbuf, &size);
  status = cirpp_load_internal(netlist_file, 0, output);
  fclose(output);
  if ( SUCCESS != status )
    goto end;
  status = cirpp(tmpbuf, outputp);
  
 end:
  free(tmpbuf);
  return status;

}
