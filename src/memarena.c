/* If this causes a segfault I will delete the entire repo */
void* f_arena_push(struct t_mem_arena a, unsigned int sz) {
	unsigned int iinc = sz / sizeof(t_arena_cell) + !!(sz % sizeof(t_arena_cell));
	return a.cur + iinc >= a.size ? (void*)0 : &a.mem[a.cur];
}


