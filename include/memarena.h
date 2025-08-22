#ifndef __H__MEMARENA_H___
#define __H__MEMARENA_H___

/* This should have correct alignment for almost any type */
/* Wrapping in struct to avoid implicit cast to pointer */
typedef struct _t_arena_aligner {
	union __t_arena_aligner {
		long double f;
		long long int i;
		void* v;
	} cell;
} t_arena_cell;

/* this struct should autoalign to t_arena_cell */
struct t_mem_arena {
	unsigned int size;
	unsigned int cur;
	t_arena_cell mem[1];
};

void* f_arena_push(struct t_mem_arena*, unsigned int);
#endif

