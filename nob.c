#define NOB_IMPLEMENTATION
#include "nob.h"

#include <limits.h>
#include <string.h>

#define _LEN(x) (sizeof ((x))/ sizeof (*(x)))
#define _STRARR(...) (const char* []) {__VA_ARGS__}
#define CHECK_REBUILD(output, ...) nob_needs_rebuild(output, _STRARR(__VA_ARGS__), _LEN( (_STRARR(__VA_ARGS__)) ) )

#define M_CC "gcc", "-Wall", "-Wextra", "-Wpedantic", "-Wswitch", "-Wvla"
#define M_OBJS "obj/window.o", "obj/render.o", "obj/vector.o", "obj/main.o"
#define M_LFLAGS "-lm", "-lglfw", "-lepoxy"
#define M_OBJCOMP "-c", "-I", "include"

void _check_assert(int assertion, char* name) {
	if(assertion)
		printf("[ASSERT] %s : passed, continuing...\n", name);
	else
		printf("[ASSERT] %s : FAILED! Exiting...\n", name), exit(-1);
}

#define chk_assert(x) _check_assert(x, #x)

int main(int argc, char* argv[]) {
	NOB_GO_REBUILD_URSELF_PLUS(argc, argv, "nob.h");

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

	/* Bit width checks for OpenGL */
	chk_assert(sizeof(float) == 4);
	chk_assert(CHAR_BIT == 8);

	putchar('\n');

	/* Check for updates and recompile object files */
	nob_cmd_append(&cmd, M_CC, M_OBJCOMP, "src/main.c", "-o", "obj/main.o");
	if(CHECK_REBUILD("obj/main.o", "src/main.c", "include/window.h")) {
		if(!nob_cmd_run_sync(cmd)) return -1;
	}
	cmd.count = 0;

	nob_cmd_append(&cmd, M_CC, M_OBJCOMP, "src/window.c", "-o", "obj/window.o");
	if(CHECK_REBUILD("obj/window.o", "src/window.c", "include/window.h")) {
		if(!nob_cmd_run_sync(cmd)) return -1;
	}
	cmd.count = 0;

	nob_cmd_append(&cmd, M_CC, M_OBJCOMP, "src/vector.c", "-o", "obj/vector.o");
	if(CHECK_REBUILD("obj/vector.o", "src/vector.c", "include/vector.h")) {
		if(!nob_cmd_run_sync(cmd)) return -1;
	}
	cmd.count = 0;

	nob_cmd_append(&cmd, M_CC, M_OBJCOMP, "src/render.c", "-o", "obj/render.o");
	if(CHECK_REBUILD("obj/render.o", "src/render.c", "include/window.h", "include/vector.h")) {
		if(!nob_cmd_run_sync(cmd)) return -1;
	}
	cmd.count = 0;

	/* Recompile final executable from objects */
	nob_cmd_append(&cmd, M_CC, M_LFLAGS, M_OBJS, "-o", "render");
	if(CHECK_REBUILD("render", M_OBJS)) {
		if(!nob_cmd_run_sync(cmd)) return -1;
	}
	cmd.count = 0;

	nob_cmd_free(cmd);
	return 0;
}

