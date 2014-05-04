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
#define _GNU_SOURCE 1
#include <stdlib.h>
#include <search.h>
#include "yana_map.h"

yana_map_t *yana_map_new(yana_map_cmp_cb compar_func,
			 yana_map_free_cb free_func)
{
  yana_map_t *map = malloc(sizeof(*map));
  if (NULL == map)
    return NULL;
  map->compar = compar_func;
  map->free = free_func;
  map->root = NULL;
  return map;
}

void yana_map_free(yana_map_t *map)
{
  tdestroy(map->root, (void (*)(void *))(map->free));
  free(map);
}

yana_pair_t *yana_pair_new(void *key, void *value)
{
  yana_pair_t *pair = malloc(sizeof(*pair));
  if ( NULL == pair )
    return NULL;
  pair->key = key;
  pair->value = value;
  return pair;
}

yana_pair_t * yana_map_set(yana_map_t *map, yana_pair_t *pair)
{
  yana_pair_t **pairp;
  if ( NULL == pair )
    return NULL;
  pairp = tsearch(pair, &(map->root),
		  (int(*)(const void*, const void*))(map->compar));
  if ( *pairp != pair )
    {
      void *tmp = (*pairp)->value;
      (*pairp)->value = pair->value;
      pair->value = tmp;
      map->free(pair);
    }
  return *pairp;
}

const yana_pair_t *yana_map_get(const yana_map_t *map, const void *key)
{
  const yana_pair_t lookup = {.key = (void*)key };
  const yana_pair_t **res = tfind(&lookup, &(map->root),
				  (int(*)(const void*, const void*))(map->compar));
  if ( NULL == res ) return NULL;
  return *res;
}

void yana_map_remove(yana_map_t *map, const void *key)
{
  const yana_pair_t lookup = { .key = (void*)key };
  tdelete(&lookup, &(map->root),
	  (int(*)(const void*, const void*))(map->compar));
}
