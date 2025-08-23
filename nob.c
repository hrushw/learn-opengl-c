#define NOB_IMPLEMENTATION
#include "nob.h"

#include <limits.h>
#include <string.h>

#include "include/memarena.h"

#define _LEN(x) (sizeof ((x))/ sizeof (*(x)))
#define _STRARR(...) (const char* []) {__VA_ARGS__}
#define CHECK_REBUILD(output, ...) nob_needs_rebuild(output, _STRARR(__VA_ARGS__), _LEN( (_STRARR(__VA_ARGS__)) ) )
#define CHECK_REBUILD_WITH_NOB(output, ...) CHECK_REBUILD(output, "nob.c", "nob.h", __VA_ARGS__)

#define DEBUG 0

#if DEBUG
	#define M_CC "gcc", "-Wall", "-Wextra", "-Wpedantic", "-Wswitch", "-Wvla", "-g"
#else
	#define M_CC "gcc", "-Wall", "-Wextra", "-Wpedantic", "-Wswitch", "-Wvla"
#endif

#define M_OBJS "obj/window.o", "obj/render.o", "obj/vector.o", "obj/memarena.o", "obj/main.o"
#define M_LFLAGS "-lm", "-lglfw", "-lepoxy"
#define M_OBJCOMP "-c", "-I", "include"

void _check_assert(int assertion, char* name) {
	if(assertion)
		printf("[ASSERT] %s : passed, continuing...\n", name);
	else
		printf("[ASSERT] %s : FAILED! Exiting...\n", name), exit(-1);
}

#define chk_assert(x) _check_assert(x, #x)

void c_checks(void) {
	/* I will probably not compile outside of x86_64 for a long time but *
	 * these cheks may save some nerd centuries in the future attempting *
	 * to compile this on some esoteric hardware */

	/* I frequently use integers as array indices, and checking  *
	 * for all such cases and replacing with size_t just in case *
	 * of 16 bit integers is too much of a pain */
	chk_assert(sizeof(int) >= 4);

	/* OpenGL requires 32 bit floats */
	chk_assert(sizeof(float) == 4);

	chk_assert(CHAR_BIT == 8);

	printf("sizeof t_arena_cell == %d\n", sizeof(t_arena_cell));

	chk_assert(sizeof(struct t_mem_arena) % sizeof(t_arena_cell) == 0);
}

int main(int argc, char* argv[]) {
	NOB_GO_REBUILD_URSELF_PLUS(argc, argv, "nob.h", "include/memarena.h");

	Nob_Cmd cmd = {0};

	if(argc > 1) {
		if(!strcmp(argv[1], "clean")) {
			nob_cmd_append(&cmd, "rm", "-f", "render", M_OBJS);
			return nob_cmd_run_sync(cmd) ? 0 : -1;
		} else if(!strcmp(argv[1], "run")) {
			nob_cmd_append(&cmd, "./render");
			return nob_cmd_run_sync(cmd) ? 0 : -1;
		} else {
			nob_log(NOB_ERROR, "Invalid option '%s'", argv[1]);
			return -1;
		}
	}

	putchar('\n');
	c_checks();
	putchar('\n');

	/* Check for updates and recompile object files */
	nob_cmd_append(&cmd, M_CC, M_OBJCOMP, "src/main.c", "-o", "obj/main.o");
	if(CHECK_REBUILD_WITH_NOB("obj/main.o", "src/main.c", "include/window.h")) {
		if(!nob_cmd_run_sync(cmd)) return -1;
	}
	cmd.count = 0;

	nob_cmd_append(&cmd, M_CC, M_OBJCOMP, "src/window.c", "-o", "obj/window.o");
	if(CHECK_REBUILD_WITH_NOB("obj/window.o", "src/window.c", "include/window.h")) {
		if(!nob_cmd_run_sync(cmd)) return -1;
	}
	cmd.count = 0;

	nob_cmd_append(&cmd, M_CC, M_OBJCOMP, "src/vector.c", "-o", "obj/vector.o");
	if(CHECK_REBUILD_WITH_NOB("obj/vector.o", "src/vector.c", "include/vector.h")) {
		if(!nob_cmd_run_sync(cmd)) return -1;
	}
	cmd.count = 0;

	nob_cmd_append(&cmd, M_CC, M_OBJCOMP, "src/render.c", "-o", "obj/render.o");
	if(CHECK_REBUILD_WITH_NOB("obj/render.o", "src/render.c", "include/window.h", "include/vector.h", "include/memarena.h")) {
		if(!nob_cmd_run_sync(cmd)) return -1;
	}
	cmd.count = 0;

	nob_cmd_append(&cmd, M_CC, M_OBJCOMP, "src/memarena.c", "-o", "obj/memarena.o");
	if(CHECK_REBUILD_WITH_NOB("obj/memarena.o", "src/memarena.c", "include/memarena.h")) {
		if(!nob_cmd_run_sync(cmd)) return -1;
	}
	cmd.count = 0;

	/* Recompile final executable from objects */
	nob_cmd_append(&cmd, M_CC, M_LFLAGS, M_OBJS, "-o", "render");
	if(CHECK_REBUILD_WITH_NOB("render", M_OBJS)) {
		if(!nob_cmd_run_sync(cmd)) return -1;
	}
	cmd.count = 0;

	nob_cmd_free(cmd);
	return 0;
}

