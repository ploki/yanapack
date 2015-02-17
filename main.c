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

#define _GNU_SOURCE 1
#include <yanapack.h>
#include <uforth.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>


struct option options[] =
  {
    { "help", no_argument, NULL, 'h' },
    { "netlist", required_argument, NULL, 'n' },
    { "command", required_argument, NULL, 'c' },
    { "interactive", no_argument, NULL, 's' },
    { "from-frequency", required_argument, NULL, 'f' },
    { "to-frequency", required_argument, NULL, 't' },
    { "steps-per-decade", required_argument, NULL, 'p'},
    { "from-log-frequency", required_argument, NULL, 'F'},
    { "to-log-frequency", required_argument, NULL, 'T' },
    {}
  };

static void
usage(FILE *output, const char *argv0)
{
  int i;
  fprintf(output, "Yanapack: Yet Another Nodal Analysis PACKage\n"
	  "usage: %s", argv0);
  for ( i = 0 ; options[i].name ; ++i )
    {
      if ( options[i].val < 256 )
	{
	  fprintf(output, " -%c", options[i].val);
	  if ( options[i].has_arg != no_argument )
	    fputs(" ARG", output);
	}
    }
  fputs("\n", output);
  for ( i = 0 ; options[i].name ; ++i )
    {
      if ( options[i].val < 256 )
	{
	  fprintf(output, "\t-%c", options[i].val);
	}
      if ( options[i].name )
	{
	  fprintf(output, "\t--%s", options[i].name);
	}
      if ( options[i].has_arg != no_argument )
	fputs("\tARG", output);
      fputs("\n", output);
    }
}

static void
make_short_options(char *buf)
{
  int i,j;
  for ( i = 0, j = 0 ; options[i].name ; ++i )
    {
      if ( options[i].val < 256 )
	{
	  buf[j++] = options[i].val;
	  if ( options[i].has_arg != no_argument )
	    buf[j++] = ':';
	  if ( options[i].has_arg == optional_argument )
	    buf[j++] = ':';
	}
    }
}

int main(int argc, char **argv)
{
  int c;
  char short_options[1024];
  int from = 1;
  int to = 6;
  int interactive = 0;
  int steps_per_decade = 50;
  char *netlist_file=NULL;
  char *command=NULL;
  
  make_short_options(short_options);
  
  while ( -1 != ( c = getopt_long(argc, argv, short_options, options, NULL) ))
    switch (c)
      {
      case 'h':
	usage(stdout, argv[0]);
	exit(EXIT_SUCCESS);
      case 'n':
	netlist_file = strdup(optarg);
	break;
      case 'c':
	command = strdup(optarg);
	break;
      case 's':
	interactive=1;
	break;
      case 'f':
	from = strtol(optarg, 0, 0);
	break;
      case 't':
	to = strtol(optarg, 0, 0);
	break;
      case 'p':
	steps_per_decade = strtol(optarg, 0, 0);
	break;
      case 'F':
	from = strtol(optarg, 0, 0);
	break;
      case 'T':
	to = strtol(optarg, 0, 0);
	break;
      case '?':
      default:
	usage(stderr, argv[0]);
	exit(EXIT_FAILURE);
      }
  if ( optind < argc )
    {
      fprintf(stderr, "Spurious arguments");
      usage(stderr, argv[0]);
      exit(EXIT_FAILURE);
    }

  if ( NULL == netlist_file )
    {
      fprintf(stderr, "Missing netlist\n");
      usage(stderr, argv[0]);
      exit(EXIT_FAILURE);
    }

  
  simulation_context_t *sc = simulation_context_new(from, to, steps_per_decade);
  if ( NULL == sc )
    {
      fprintf(stderr, "Failed to create a simulation context\n");
      exit(EXIT_FAILURE);
    }
  struct stat st;
  int ret;
  ret = stat(netlist_file, &st);
  if (ret < 0 )
    {
      perror(netlist_file);
      exit(EXIT_FAILURE);
    }
  char *netlist_buf = malloc(st.st_size);
  if ( NULL == netlist_buf )
    {
      fprintf(stderr, "ENOMEM\n");
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
  fclose(f);

  netlist_t *netlist;
  nodelist_t *nodelist;
  simulation_t *simulation;
  status_t status;

  status = netlist_new(sc, netlist_buf, &netlist);
  if ( SUCCESS != status )
    {
      fprintf(stderr, "failed to load netlist\n");
      exit(EXIT_FAILURE);
    }
  status = nodelist_new(sc, netlist, &nodelist);
  if ( SUCCESS != status )
    {
      fprintf(stderr, "failed to create nodelist\n");
      exit(EXIT_FAILURE);
    }
  status = simulation_new(sc, nodelist, &simulation);
  if ( SUCCESS != status )
    {
      fprintf(stderr, "failed to create nodelist\n");
      exit(EXIT_FAILURE);
    }

  simulation_set_values(simulation);
  simulation_solve(simulation);

  if ( interactive )
    printf("plop\n");
  
  if ( command )
    printf("gr8k\n");

  uforth_context_t *uf_ctx;
  status = uforth_compile(netlist_buf, 1000, &uf_ctx);
  assert( SUCCESS == status );
  uforth_execute(uf_ctx, sc, simulation);
  return 0;
}
