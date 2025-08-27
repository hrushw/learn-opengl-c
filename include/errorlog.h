#ifndef __H__ERRORLOG_H___
#define __H__ERRORLOG_H___

#include "fileio.h"
#include "shader.h"

void f_error_log_f2b (
	enum e_filetobuf_error, const char*,
	unsigned int, unsigned int
);

void f_error_log_shader (
	enum e_gl_shader_check, int,
	char*, unsigned int
);

void f_error_log_program (
	enum e_gl_shader_check, char*, unsigned int
);

#endif
