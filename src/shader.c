#include <epoxy/gl.h>

#include "shader.h"

enum e_gl_shader_check f_gl_genshader(
	unsigned int *sid, int type, const char* srcbuf,
	char* logbuf, unsigned int logbufsz, unsigned int* log_len
) {
	if(!srcbuf) return ERR_GL_SHADER_FAIL;

	unsigned int s = glCreateShader(type);
	glShaderSource(s, 1, &srcbuf, NULL);
	glCompileShader(s);

	*sid = s;

	int ret = 0;
	glGetShaderiv(s, GL_COMPILE_STATUS, &ret);
	if(ret) return ERR_GL_SHADER_SUCCESS;

	if(!logbuf) return ERR_GL_SHADER_FAIL;

	int gl_il_len;
	glGetShaderiv(s, GL_INFO_LOG_LENGTH, &gl_il_len);
	if(log_len) *log_len = gl_il_len;

	if(!logbufsz) return ERR_GL_SHADER_FAIL_LOG_INCOMPLETE;

	glGetShaderInfoLog(s, logbufsz, NULL, logbuf);
	logbuf[logbufsz-1] = 0;

	return (unsigned int) gl_il_len > logbufsz ?
		ERR_GL_SHADER_FAIL_LOG_INCOMPLETE :
		ERR_GL_SHADER_FAIL ;
}

/* Generate shader program from given vertex and fragment shaders */
enum e_gl_shader_check f_gl_genprogram(
	unsigned int *spid, unsigned int vert, unsigned int frag,
	char* logbuf, unsigned int logbufsz, unsigned int* log_len
) {
	unsigned int sp = glCreateProgram();
	glAttachShader(sp, vert);
	glAttachShader(sp, frag);
	glLinkProgram(sp);

	*spid = sp;

	int ret = 0;
	glGetProgramiv(sp, GL_LINK_STATUS, &ret);
	if(ret) return ERR_GL_SHADER_SUCCESS;

	if(!logbuf) return ERR_GL_SHADER_FAIL;

	int gl_il_len;
	glGetProgramiv(sp, GL_INFO_LOG_LENGTH, &gl_il_len);
	if(log_len) *log_len = gl_il_len;

	if(!logbufsz) return ERR_GL_SHADER_FAIL_LOG_INCOMPLETE;

	glGetProgramInfoLog(sp, logbufsz, NULL, logbuf);
	logbuf[logbufsz-1] = 0;

	return (unsigned int) gl_il_len > logbufsz ?
		ERR_GL_SHADER_FAIL_LOG_INCOMPLETE :
		ERR_GL_SHADER_FAIL ;
}

