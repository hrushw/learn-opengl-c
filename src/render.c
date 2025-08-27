#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <epoxy/gl.h>

#include <stdio.h>

#include "window.h"
#include "fileio.h"
#include "shader.h"
#include "errorlog.h"

void f_render_main(void* win) {
	enum e_bufsz_src { BUFSZ_SRC = 0x2000 };
	enum e_bufsz_log { BUFSZ_LOG = 0x1000 };

	static char chbuf[ 2*BUFSZ_SRC + 3*BUFSZ_LOG ] = {0};

	char* vert_src = chbuf;
	char* frag_src = chbuf + BUFSZ_SRC;
	char* vert_log = chbuf + 2*BUFSZ_SRC;
	char* frag_log = chbuf + 2*BUFSZ_SRC + BUFSZ_LOG;
	char* prog_log = chbuf + 2*BUFSZ_SRC + 2*BUFSZ_LOG;

	unsigned int len = 0;
	const char* path;
	unsigned int ret;

	path = "shaders/vertex.glsl";
	ret = f_io_filetobuf(path, &len, vert_src, BUFSZ_SRC);
	f_error_log_f2b(ret, path, BUFSZ_SRC, len);

	unsigned int vert = 0;
	ret = f_gl_genshader(&vert, GL_VERTEX_SHADER, vert_src, vert_log, BUFSZ_LOG, &len);
	f_error_log_shader(ret, GL_VERTEX_SHADER, vert_log, len);

	path = "shaders/fragment.glsl";
	ret = f_io_filetobuf(path, &len, frag_src, BUFSZ_SRC);
	f_error_log_f2b(ret, path, BUFSZ_SRC, len);

	unsigned int frag = 0;
	ret = f_gl_genshader(&frag, GL_FRAGMENT_SHADER, frag_src, frag_log, BUFSZ_LOG, &len);
	f_error_log_shader(ret, GL_FRAGMENT_SHADER, frag_log, len);

	unsigned int sp = 0;
	ret = f_gl_genprogram(&sp, vert, frag, prog_log, BUFSZ_LOG, &len);
	f_error_log_program(ret, prog_log, len);

	struct t_glfw_winstate* wst = glfwGetWindowUserPointer(win);

	for(glfwSetTime(0.0); wst->runstate; wst->time = glfwGetTime()) {

		glfwSwapBuffers(win);
		glfwPollEvents();

		if(wst->iqoverflow) {
			fprintf(stderr,
				"ERROR: Key press queue indices out of bounds! clearing...\n"
				"(start index = %d, end index = %d, max queue size = %d)\n",
				wst->iq.start, wst->iq.end, IQSZ_
			);

			wst->iq.start = 0;
			wst->iq.end = 0;
			wst->iqoverflow = 0;
		}
	}
}
