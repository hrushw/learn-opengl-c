#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#include "nob.h"

#include "include/window.h"

#include <string.h>
#include <stddef.h>

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

#define M_OBJS "obj/window.o", "obj/render.o", "obj/vector.o", "obj/shader.o", "obj/main.o", "obj/fileio.o", "obj/errorlog.o"
#define M_HEADERS "include/vector.h", "include/window.h", "include/shader.h", "include/fileio.h", "include/errorlog.h"
#define M_LFLAGS "-lm", "-lglfw", "-lepoxy"
#define M_OBJCOMP "-c", "-I", "include"

void _die(const char* msg, int ret) {
	fprintf(stderr, "%s\n", msg);
	exit(ret);
}

void try_run(Nob_Cmd *cmd) {
	if(!nob_cmd_run(cmd)) exit(-1);
}

#define _TEST(chk) _chk_assert(chk, #chk)

void _chk_assert(int chk, const char* msg) {
	if(chk)
		fprintf(stderr, "[Check]: %s passed, continuing...\n", msg);
	else
		fprintf(stderr, "[Check]: %s FAILED, exiting...\n", msg), exit(-1);
}

int main(int argc, char* argv[]) {
	NOB_GO_REBUILD_URSELF_PLUS(argc, argv, "nob.h", "include/window.h");

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


	putchar('\n');
	_TEST(CHAR_BIT == 8);
	_TEST(sizeof(float) == 4);
	_TEST(sizeof(unsigned int) >= 4);
	putchar('\n');
	printf("sizeof(struct t_glfw_winstate) = %lu\n", sizeof(struct t_glfw_winstate));
	printf("sizeof(struct t_glfw_inputevent) = %lu\n", sizeof(struct t_glfw_inputevent));
	printf("sizeof(union t_glfw_inputevent_u_) = %lu\n", sizeof(union t_glfw_inputevent_u_));
	putchar('\n');

	/* Check for updates and recompile object files */
	if(CHECK_REBUILD_WITH_NOB("obj/main.o", "src/main.c", "include/window.h")) {
		nob_cmd_append(&cmd, M_CC, M_OBJCOMP, "src/main.c", "-o", "obj/main.o");
		try_run(&cmd);
	}

	if(CHECK_REBUILD_WITH_NOB("obj/window.o", "src/window.c", "include/window.h")) {
		nob_cmd_append(&cmd, M_CC, M_OBJCOMP, "src/window.c", "-o", "obj/window.o");
		try_run(&cmd);
	}

	if(CHECK_REBUILD_WITH_NOB("obj/vector.o", "src/vector.c", "include/vector.h")) {
		nob_cmd_append(&cmd, M_CC, M_OBJCOMP, "src/vector.c", "-o", "obj/vector.o");
		try_run(&cmd);
	}

	if(CHECK_REBUILD_WITH_NOB("obj/shader.o", "src/shader.c", "include/shader.h")) {
		nob_cmd_append(&cmd, M_CC, M_OBJCOMP, "src/shader.c", "-o", "obj/shader.o");
		try_run(&cmd);
	}

	if(CHECK_REBUILD_WITH_NOB("obj/fileio.o", "src/fileio.c", "include/fileio.h")) {
		nob_cmd_append(&cmd, M_CC, M_OBJCOMP, "src/fileio.c", "-o", "obj/fileio.o");
		try_run(&cmd);
	}

	if(CHECK_REBUILD_WITH_NOB("obj/errorlog.o", "src/errorlog.c", "include/errorlog.h", "include/fileio.h", "include/shader.h")) {
		nob_cmd_append(&cmd, M_CC, M_OBJCOMP, "src/errorlog.c", "-o", "obj/errorlog.o");
		try_run(&cmd);
	}

	if(CHECK_REBUILD_WITH_NOB("obj/render.o", "src/render.c", M_HEADERS)) {
		nob_cmd_append(&cmd, M_CC, M_OBJCOMP, "src/render.c", "-o", "obj/render.o");
		try_run(&cmd);
	}

	/* Recompile final executable from objects */
	if(CHECK_REBUILD_WITH_NOB("render", M_OBJS)) {
		nob_cmd_append(&cmd, M_CC, M_LFLAGS, M_OBJS, "-o", "render");
		try_run(&cmd);
	}

	nob_cmd_free(cmd);
	return 0;
}
