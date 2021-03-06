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

#ifndef __YANAPACK_UFORTH_H__
#define __YANAPACK_UFORTH_H__

#include <yanapack.h>

typedef struct uforth_context uforth_context_t;
typedef struct uforth_heap uforth_heap_t;

status_t uforth_compile(const char *buf, int stack_size,
			uforth_context_t **uf_ctxp);
status_t
uforth_compile_command(const char *buf, int stack_size,
		       uforth_context_t **uf_ctxp);

void uforth_free(uforth_context_t *uf_ctx);

status_t
uforth_execute(uforth_context_t *uf_ctx,
	       simulation_context_t *sc,
	       simulation_t *simulation,
	       uforth_heap_t *heap,
	       yana_complex_t *resultp);

uforth_heap_t *
uforth_heap_new(void);
void
uforth_heap_free(uforth_heap_t *heap);


#endif /* __YANAPACK_UFORTH_H__ */
