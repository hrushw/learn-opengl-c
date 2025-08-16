#define NOB_IMPLEMENTATION
#include "nob.h"

#include <string.h>

#define _LEN(x) (sizeof ((x))/ sizeof (*(x)))
#define _STRARR(...) (const char* []) {__VA_ARGS__}
#define CHECK_REBUILD(output, ...) nob_needs_rebuild(output, _STRARR(__VA_ARGS__), _LEN( (_STRARR(__VA_ARGS__)) ) )

#define M_CC "gcc", "-Wall", "-Wextra", "-Wpedantic", "-Wswitch", "-Wvla"
#define M_OBJS "obj/window.o", "obj/render.o", "obj/vector.o", "obj/main.o"
#define M_LFLAGS "-lm", "-lglfw", "-lepoxy"
#define M_OBJCOMP "-c", "-I", "include"

int main(int argc, char* argv[]) {
	NOB_GO_REBUILD_URSELF(argc, argv);
	Nob_Cmd cmd = {0};

	if(argc > 1) {
		if(!strcmp(argv[1], "clean")) {
			nob_cmd_append(&cmd, "rm", "-f", "render", M_OBJS);
			if(nob_cmd_run_sync_and_reset(&cmd)) return -1;
			return 0;
		} else {
			printf("Invalid option '%s'\n", argv[1]);
		}
	}

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

	nob_cmd_append(&cmd, M_CC, M_LFLAGS, M_OBJS, "-o", "render");
	if(CHECK_REBUILD("render", M_OBJS)) {
		if(!nob_cmd_run_sync(cmd)) return -1;
	}
	cmd.count = 0;

	nob_cmd_free(cmd);
	return 0;
}

