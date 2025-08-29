#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#include "nob.h"

#include <string.h>

#define _LEN(x) (sizeof ((x))/ sizeof (*(x)))
#define _STRARR(...) (const char* []) {__VA_ARGS__}
#define CHECK_REBUILD(output, ...) nob_needs_rebuild(output, _STRARR(__VA_ARGS__), _LEN( (_STRARR(__VA_ARGS__)) ) )
#define CHECK_REBUILD_WITH_NOB(output, ...) CHECK_REBUILD(output, "nob.c", "nob.h", __VA_ARGS__)
#define CMD_APPEND_RUN_RESET(cmd, ...) { nob_cmd_append(cmd, __VA_ARGS__); if(!nob_cmd_run(cmd)) _die("ERROR: Command failed!\n", -1); }

#define DEBUG 0

#if DEBUG
	#define M_CC "gcc", "-Wall", "-Wextra", "-Wpedantic", "-Wswitch", "-Wvla", "-g"
#else
	#define M_CC "gcc", "-Wall", "-Wextra", "-Wpedantic", "-Wswitch", "-Wvla"
#endif

#define M_OBJS "obj/window.o", "obj/render.o", "obj/vector.o", "obj/shader.o", "obj/main.o", "obj/fileio.o", "obj/errorlog.o"
#define M_HEADERS "include/vector.h", "include/window.h", "include/shader.h", "include/fileio.h", "include/errorlog.o"
#define M_LFLAGS "-lm", "-lglfw", "-lepoxy"
#define M_OBJCOMP "-c", "-I", "include"

void _die(const char* msg, int ret) {
	fprintf(stderr, "%s\n", msg);
	exit(ret);
}

int main(int argc, char* argv[]) {
	NOB_GO_REBUILD_URSELF_PLUS(argc, argv, "nob.h");

	Nob_Cmd cmd = {0};

	if(argc > 1) {
		if(!strcmp(argv[1], "clean")) {
			nob_cmd_append(&cmd, "rm", "-f", "render", M_OBJS);
			return nob_cmd_run(&cmd) ? 0 : -1;
		} else if(!strcmp(argv[1], "run")) {
			nob_cmd_append(&cmd, "./render");
			return nob_cmd_run(&cmd) ? 0 : -1;
		} else {
			nob_log(NOB_ERROR, "Invalid option '%s'", argv[1]);
			return -1;
		}
	}

	/* Check for updates and recompile object files */
	if(CHECK_REBUILD_WITH_NOB("obj/main.o", "src/main.c", "include/window.h"))
		CMD_APPEND_RUN_RESET(&cmd, M_CC, M_OBJCOMP, "src/main.c", "-o", "obj/main.o")

	if(CHECK_REBUILD_WITH_NOB("obj/window.o", "src/window.c", "include/window.h"))
		CMD_APPEND_RUN_RESET(&cmd, M_CC, M_OBJCOMP, "src/window.c", "-o", "obj/window.o")

	if(CHECK_REBUILD_WITH_NOB("obj/vector.o", "src/vector.c", "include/vector.h"))
		CMD_APPEND_RUN_RESET(&cmd, M_CC, M_OBJCOMP, "src/vector.c", "-o", "obj/vector.o")

	if(CHECK_REBUILD_WITH_NOB("obj/shader.o", "src/shader.c", "include/shader.h"))
		CMD_APPEND_RUN_RESET(&cmd, M_CC, M_OBJCOMP, "src/shader.c", "-o", "obj/shader.o")

	if(CHECK_REBUILD_WITH_NOB("obj/fileio.o", "src/fileio.c", "include/fileio.h"))
		CMD_APPEND_RUN_RESET(&cmd, M_CC, M_OBJCOMP, "src/fileio.c", "-o", "obj/fileio.o")

	if(CHECK_REBUILD_WITH_NOB("obj/errorlog.o", "src/errorlog.c", "include/errorlog.h", "include/fileio.h", "include/shader.h"))
		CMD_APPEND_RUN_RESET(&cmd, M_CC, M_OBJCOMP, "src/errorlog.c", "-o", "obj/errorlog.o")

	if(CHECK_REBUILD_WITH_NOB("obj/render.o", "src/render.c", M_HEADERS))
		CMD_APPEND_RUN_RESET(&cmd, M_CC, M_OBJCOMP, "src/render.c", "-o", "obj/render.o")

	/* Recompile final executable from objects */
	if(CHECK_REBUILD_WITH_NOB("render", M_OBJS))
		CMD_APPEND_RUN_RESET(&cmd, M_CC, M_LFLAGS, M_OBJS, "-o", "render")

	nob_cmd_free(cmd);
	return 0;
}
