#ifndef __H__SHADER_H___
#define __H__SHADER_H___

enum e_gl_shader_check {
	ERR_GL_SHADER_SUCCESS = 0,
	ERR_GL_SHADER_FAIL,
	ERR_GL_SHADER_FAIL_LOG_INCOMPLETE,
};

enum e_gl_shader_check f_gl_genshader(
	unsigned int *, int, const char*,
	char*, unsigned int, unsigned int*
);

enum e_gl_shader_check f_gl_genprogram(
	unsigned int*, unsigned int, unsigned int,
	char*, unsigned int, unsigned int*
);

#endif
