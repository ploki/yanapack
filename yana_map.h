#ifndef __YANAPACK_MAP_H__
#define __YANAPACK_MAP_H__ 1

typedef struct 
{
  void *key;
  void *value;
} yana_pair_t;


typedef int (*yana_map_cmp_cb)(const yana_pair_t *lhs, const yana_pair_t *rhs);
typedef void (*yana_map_free_cb)(yana_pair_t *pair);

typedef struct
{
  void *root;
  yana_map_cmp_cb compar;
  yana_map_free_cb free;
} yana_map_t;

yana_map_t *yana_map_new(yana_map_cmp_cb compar_func,
			 yana_map_free_cb free_func);
void yana_map_free(yana_map_t *map);
yana_pair_t *yana_pair_new(void *key, void *value);
yana_pair_t *yana_map_set(yana_map_t *map, yana_pair_t *pair);
const yana_pair_t *yana_map_get(const yana_map_t *map, const void *key);
void yana_map_remove(yana_map_t *map, const void *key);

#endif
