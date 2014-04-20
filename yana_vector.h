#ifndef __YANAPACK_VECTOR_H__
#define __YANAPACK_VECTOR_H__ 1
#include <assert.h>
#include <stdlib.h>

#define VEC_T(TYPE) vec_ ## TYPE ## _t

#define USE_VEC_T(TYPE)							\
  typedef struct vec_ ## TYPE ## _t					\
  {									\
    size_t count;							\
    size_t size;							\
    TYPE ## _t *elems;							\
  } vec_ ## TYPE ## _t;							\
									\
									\
  __attribute__((unused))						\
  static inline vec_ ## TYPE ## _t *					\
  vec_ ## TYPE ## _new(size_t initial_size)				\
  {									\
    if ( 0 == initial_size )						\
      initial_size = 1;							\
    vec_ ## TYPE ## _t * vec =						\
      malloc( sizeof(vec_ ## TYPE ## _t));				\
      vec->elems = malloc( initial_size * sizeof( TYPE ## _t) );	\
      vec->count = 0;							\
      vec->size = initial_size;						\
      return vec;							\
  }									\
  __attribute__((unused))						\
  static inline TYPE ## _t *						\
  vec_ ## TYPE ## _at(vec_ ## TYPE ## _t * vec, size_t idx)		\
  {									\
    assert( idx < vec->count );						\
    return &vec->elems[idx];						\
  }									\
  __attribute__((unused))						\
  static inline TYPE ## _t *						\
  vec_ ## TYPE(vec_ ## TYPE ## _t *vec)					\
  {									\
    return vec->elems;							\
  }									\
									\
									\
  __attribute__((unused))						\
  static inline void							\
  vec_ ## TYPE ## _grow(vec_ ## TYPE ## _t *vec)			\
  {									\
    size_t new_size = vec->size*2;					\
    void *reallocated = realloc(vec->elems,				\
				new_size * sizeof( TYPE ## _t));	\
    assert( NULL != reallocated );					\
    vec->elems = reallocated;						\
    vec->size = new_size;						\
  }									\
									\
  __attribute__((unused))						\
  static inline TYPE ## _t *						\
  vec_ ## TYPE ## _push_back(vec_ ## TYPE ## _t *vec, TYPE ## _t elem)	\
  {									\
    TYPE ## _t *ret;							\
      if ( vec->count == vec->size )					\
	vec_ ## TYPE ## _grow(vec);					\
	  vec->elems[vec->count] = elem;				\
	  ret = &vec->elems[vec->count];				\
	  ++vec->count;							\
	  return ret;							\
  }									\
									\
  __attribute__((unused))						\
  static inline size_t							\
  vec_ ## TYPE ## _count(vec_ ## TYPE ## _t *vec)			\
  {									\
    return vec->count;							\
  }									\
  __attribute__((unused))						\
  static inline void							\
  vec_ ## TYPE ## _free(vec_ ## TYPE ## _t *vec)			\
  {									\
    free(vec->elems);							\
    free(vec);								\
  }


#endif
