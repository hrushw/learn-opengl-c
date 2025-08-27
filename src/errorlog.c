#include <epoxy/gl.h>
#include <stdio.h>

#include "errorlog.h"

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
			"ERROR: Cannot read file '%s' into buffer!\n"
			"Cause: not enough space!\n"
			"[ buffer length = %u, file size = %u ]\n",
			path, buflen, flen
		);
		break;

	case ERR_F2B_FAILED_READ:
		fprintf(stderr,
			"ERROR: Cannot read file '%s' into buffer!\n"
			"Cause: read operation failed!\n",
			path
		);
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
		fprintf(stderr,
			"ERROR: Failed to compile %s shader!\n[ERROR LOG]:\n%s\n",
			typestr, logbuf
		);
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

void f_error_log_program(
		enum e_gl_shader_check ret, char* logbuf, unsigned int loglen
) {
	switch(ret) {
	case ERR_GL_SHADER_FAIL:
		fprintf(stderr,
			"ERROR: Failed to link program!\n[ERROR LOG]:\n%s\n",
			logbuf
		);
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

