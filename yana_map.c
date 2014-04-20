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
