#ifndef __H__MEMARENA_H___
#define __H__MEMARENA_H___

/* This should have correct alignment for almost any type */
typedef union _t_arena_aligner {
	long double f;
	long long int i;
	void* v;
} t_arena_cell;

struct t_mem_arena {
	unsigned int size;
	unsigned int cur;
	t_arena_cell *mem;
};

#endif

