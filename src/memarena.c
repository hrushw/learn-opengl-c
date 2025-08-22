#include "memarena.h"

/* pls dont segfault */
void* f_arena_push(struct t_mem_arena *a, unsigned int sz) {
	void* ret = 0;

	unsigned int new = a->cur + ( sz / sizeof(t_arena_cell) + !!( sz % sizeof(t_arena_cell) ) );

	return new >= a->size ? (
		ret = (void*)0, ret
	) : (
		ret = &a->mem[a->cur], a->cur = new, ret
	);
}


