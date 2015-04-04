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
