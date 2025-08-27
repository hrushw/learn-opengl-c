#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <epoxy/gl.h>

#include <stdio.h>

#include "window.h"
#include "fileio.h"
#include "shader.h"

void f_error_log_f2b(enum e_filetobuf_error ret, const char* path, unsigned int buflen, unsigned int flen) {
	switch(ret & ~ERR_F2B_FAILED_CLOSE) {
	case ERR_F2B_ZERO_SIZE_BUFFER:
		fprintf(stderr, "ERROR: Cannot read into zero sized buffer!\n");
		break;

	case ERR_F2B_FAILED_OPEN:
		fprintf(stderr, "ERROR: Failed to open file '%s'!\n", path);
		break;

	case ERR_F2B_FAILED_SEEK:
		fprintf(stderr, "ERROR: Failed to seek to end of file '%s'!\n", path);
		break;

	case ERR_F2B_FAILED_GET_SIZE:
		fprintf(stderr, "ERROR: Failed to get size of file '%s'!\n", path);
		break;

	case ERR_F2B_BUFFER_TOO_SMALL:
		fprintf (stderr,
			"ERROR: Cannot read file '%s' into buffer - not enough space!\n"
			"[ buffer length = %u, file size = %u ]\n",
			path, buflen, flen
		);
		break;

	case ERR_F2B_FAILED_READ:
		fprintf(stderr, "ERROR: Cannot read file '%s' into buffer - read operation failed!\n", path);
		break;

	case ERR_F2B_SUCCESS:
		fprintf(stderr, "[LOG]: Successfully read file '%s' into buffer!\n", path);
		break;

	default:
		fprintf(stderr, "ERROR: Invalid return from function 'f_io_filetobuf'!\n");
	}

	if(ret & ERR_F2B_FAILED_CLOSE)
		fprintf(stderr, "ERROR: Failed to close file '%s'\n", path);
}

void f_error_log_shader(enum e_gl_shader_check ret, int type, char* logbuf, unsigned int loglen) {
	const char* typestr;
	switch(type) {
	case GL_VERTEX_SHADER:
		typestr = "vertex";
		break;

	case GL_FRAGMENT_SHADER:
		typestr = "fragment";
		break;

	default:
		typestr = "[[unknown]]";
	};

	switch(ret) {
	case ERR_GL_SHADER_FAIL:
		fprintf(stderr, "ERROR: Failed to compile %s shader!\n[ERROR LOG]:\n%s\n", typestr, logbuf);
		break;

	case ERR_GL_SHADER_FAIL_LOG_INCOMPLETE:
		fprintf(stderr,
			"ERROR: Failed to compile %s shader!\n"
			"[ERROR LOG][INCOMPLETE, full size = %d bytes]:\n%s\n",
			typestr, loglen, logbuf
		);
		break;

	case ERR_GL_SHADER_SUCCESS:
		fprintf(stderr, "[LOG]: Successfully compiled %s shader!\n", typestr);
		break;

	default:
		fprintf(stderr, "ERROR: Invalid return from function 'f_gl_genshader'!\n");
	}
}

void f_error_log_program(enum e_gl_shader_check ret, char* logbuf, unsigned int loglen) {
	switch(ret) {
	case ERR_GL_SHADER_FAIL:
		fprintf(stderr, "ERROR: Failed to link program!\n[ERROR LOG]:\n%s\n", logbuf);
		break;

	case ERR_GL_SHADER_FAIL_LOG_INCOMPLETE:
		fprintf(stderr,
			"ERROR: Failed to link program!\n"
			"[ERROR LOG][INCOMPLETE, full size = %d bytes]:\n%s\n",
			loglen, logbuf
		);
		break;

	case ERR_GL_SHADER_SUCCESS:
		fprintf(stderr, "[LOG]: Successfully linked program!\n");
		break;

	default:
		fprintf(stderr, "ERROR: Invalid return from function 'f_gl_genprogram'!\n");
	}
}

void f_render_main(void* win) {
	enum e_bufsz_src { BUFSZ_SRC = 0x2000 };
	enum e_bufsz_log { BUFSZ_LOG = 0x1000 };

	char chbuf[ 2*BUFSZ_SRC + 3*BUFSZ_LOG ] = {0};

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
