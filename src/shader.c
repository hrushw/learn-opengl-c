#include <epoxy/gl.h>

#include <stdio.h>

/* OpenGL shader creation wrappers */

/* Read file into buffer */
/* Returns length 0 on any failure, always null terminated */
void f_io_filetobuf(const char* path, int* len, char* buf, int buflen) {
	FILE* const f = fopen(path, "rb");
	long l = 0;

	if(!f) {
		fprintf(stderr, "ERROR: Failed to open file '%s'!\n", path);
		return;
	} else if( fseek(f, 0L, SEEK_END) == -1 )
		fprintf(stderr, "ERROR: Failed to seek to end of file '%s'!\n", path);
	else if( (l = ftell(f)) < 0 )
		fprintf(stderr, "ERROR: Failed to get size of file '%s'\n", path);
	else if( l > buflen - 1 )
		fprintf(stderr, "ERROR: File '%s' too large!\n(max size = %d bytes)\n", path, buflen - 1);
	else if( rewind(f), fread(buf, sizeof(char), l, f) != (size_t)l )
		fprintf(stderr, "ERROR: Error occured while reading file '%s'!\n", path);
	else goto finish;

	/* On any read failures return empty string */
	l = 0;

	/* Always return null-terminated string */
	finish:
	buf[l] = 0;
	if(len) *len = l;
	if(fclose(f)) fprintf(stderr, "ERROR: Unable to close file '%s'\n", path);
}

/* Check if shader was compiled successfully */
void f_gl_chkcmp(unsigned int s, char* infolog, int il_len) {
	int success = 0;
	glGetShaderiv(s, GL_COMPILE_STATUS, &success);
	if(success) return;

	int gl_il_len;
	glGetShaderiv(s, GL_INFO_LOG_LENGTH, &gl_il_len);
	if(gl_il_len > il_len)
		fprintf(stderr,
			"ERROR: Unable to get complete shader info log - log too large!\n"
			"(size = %d, max size = %d)\n",
			gl_il_len, il_len
		);

	glGetShaderInfoLog(s, il_len, NULL, infolog);
	infolog[il_len-1] = 0;
	fprintf(stderr, "ERROR: Failed to compile shader! Error log:\n%s\n", infolog);
}

/* Generate shader from file path - general function */
unsigned int f_gl_genshader(const char* path, int type, char* chbuf, int chbufsz) {
	int len = 0;
	f_io_filetobuf(path, &len, chbuf, chbufsz);

	const unsigned int s = glCreateShader(type);
	glShaderSource(s, 1, (const char* const*)(&chbuf), NULL);
	glCompileShader(s);

	f_gl_chkcmp(s, chbuf, chbufsz);
	return s;
}

/* Check if program was linked successfully */
void f_gl_chklink(unsigned int sp, char* infolog, int il_len) {
	int success = 0;
	glGetProgramiv(sp, GL_LINK_STATUS, &success);
	if(success) return;

	int gl_il_len;
	glGetProgramiv(sp, GL_INFO_LOG_LENGTH, &gl_il_len);
	if(gl_il_len > il_len)
		fprintf(stderr,
			"ERROR: Unable to get complete shader program info log - log too large!\n"
			"(size = %d, max size = %d)\n",
			gl_il_len, il_len
		);

	glGetProgramInfoLog(sp, il_len, NULL, infolog);
	infolog[il_len-1] = 0;
	fprintf(stderr, "ERROR: Unable to link shader program! Error log:\n%s\n", infolog);
}

/* Generate shader program from given vertex and fragment shaders */
unsigned int f_gl_genprogram_vf(
	unsigned int vert,
	unsigned int frag,
	char* chbuf,
	int chbufsz
) {
	unsigned int sp = glCreateProgram();

	glAttachShader(sp, vert);
	glAttachShader(sp, frag);

	glLinkProgram(sp);

	f_gl_chklink(sp, chbuf, chbufsz);
	return sp;
}


