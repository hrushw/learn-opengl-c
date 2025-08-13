#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <epoxy/gl.h>

#include <stdio.h>
#include <math.h>

#include "window.h"
#include "vector.h"

/* References
 * OpenGL Tutorials (Victor Gordan) - https://www.youtube.com/playlist?list=PLPaoO-vpZnumdcb4tZc4x5Q-v7CkrQ6M-
 * OpenGL for Beginners (OGLDEV) - https://www.youtube.com/playlist?list=PLA0dXqQjCx0S04ntJKUftl6OaOgsiwHjA
 * LearnOpenGL (Camera) - https://learnopengl.com/Getting-started/Camera

 * OpenGL 4.6 specification - https://registry.khronos.org/OpenGL/specs/gl/glspec46.core.pdf
 * GLFW documentation [window guide] - https://www.glfw.org/docs/latest/window_guide.html
 * OpenGL wiki [Rendering Pipeline Overview] - https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview
 */

#define M_LEN(x) (sizeof(x) / sizeof(*x))

enum e_chbufsz_ { CHBUFSZ_ = 0x10000 };

/* Read file into buffer */
/* No longer causes program exit on failure */
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
		fprintf(stderr,
			"ERROR: File '%s' too large!\n"
			"(max size = %d bytes)\n",
		path, buflen - 1);
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
unsigned int f_gl_genprogram(unsigned int vert, unsigned int frag, char* chbuf, int chbufsz) {
	unsigned int sp = glCreateProgram();

	glAttachShader(sp, vert);
	glAttachShader(sp, frag);

	glLinkProgram(sp);

	f_gl_chklink(sp, chbuf, chbufsz);
	return sp;
}

/* Evaluate keyboard and mouse events */
/* Currently handles shortcuts for resetting time and exit */
void f_render_evalstate(struct t_glfw_winstate *wst) {
	for(int i = wst->iq.start; (i %= IQSZ_) != wst->iq.end; ++i) {
		struct t_glfw_inputevent *qev = &wst->iq.queue[i];

		if(
			qev->key == GLFW_KEY_T &&
			qev->mods == GLFW_MOD_CONTROL &&
			qev->action == GLFW_RELEASE
		)
			glfwSetTime(0.0), wst->time = 0.0;

		if(
			qev->key == GLFW_KEY_Q &&
			qev->mods == (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT) &&
			qev->action == GLFW_RELEASE
		)
			wst->runstate = 0;
	}

	/* Reset queue after evaluation */
	wst->iq.start = 0, wst->iq.end = 0;
}

/* xyz coordinates, rgb colors, texture coordinates  */
const float vertices[] = {
	 0.0f,  0.8f,  0.0f,    1.0f, 1.0f, 1.0f,    0.0f, 0.0f,
	 0.0f, -0.2f, -0.8f,    1.0f, 0.0f, 0.0f,    0.0f, 1.0f,
	 0.7f, -0.6f,  0.4f,    0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
	-0.7f, -0.6f,  0.4f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f
};

/* Index data for rendering as triangle strip */
const unsigned int indices[] = {
	0, 1, 2, 3, 0, 1
};


/* 4x4 RGB pixel data */
const float pixels[] = {
	0.6f, 1.0f, 0.8f,    0.5f, 0.9f, 0.6f,    1.0f, 0.2f, 0.4f,    0.6f, 1.0f, 0.8f,
	0.4f, 0.9f, 1.0f,    0.8f, 0.5f, 0.5f,    0.3f, 0.3f, 1.0f,    0.8f, 0.5f, 0.5f,
	0.6f, 1.0f, 0.8f,    0.5f, 0.9f, 0.6f,    1.0f, 0.2f, 0.4f,    0.6f, 1.0f, 0.8f,
	0.4f, 0.9f, 1.0f,    0.8f, 0.5f, 0.5f,    0.3f, 0.3f, 1.0f,    0.8f, 0.5f, 0.5f,
};

void f_render_init(void) {
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_DYNAMIC_DRAW);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_FLOAT, pixels);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));

	// glDepthRange(-1.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	// glDepthFunc(GL_LESS);

	/* Culling overrides the depth test ??? */
	// glEnable(GL_CULL_FACE);
	// glFrontFace(GL_CW);
	// glCullFace(GL_FRONT);
}

void f_render_loop(void* win, int transformloc) {
	struct t_glfw_winstate* const wst = glfwGetWindowUserPointer(win);

	struct mat4x4f transforms[] = {
		c_mat4x4f_identity,
		f_mat_perspective(M_PI/3.0, 1.0, 6.0),
		f_mat_translate(0.6f, 0.0f, 2.0f),
		c_mat4x4f_identity,
		f_mat_translate(0.4f, 0.1f, 0.0f),
		f_mat_scale(0.5f),
		c_mat4x4f_identity,
		c_mat4x4f_identity,
	};

	/* Initialize time and loop */
	for(glfwSetTime(0.0); wst->runstate; wst->time = glfwGetTime()) {
		/* Set viewport and view transform matrix */
		if(wst->szrefresh) {
			wst->szrefresh = 0;
			glViewport(0, 0, wst->width, wst->height);

			double scalex, scaley;
			if(wst->width > wst->height)
				scalex = (double)wst->height / (double)wst->width, scaley = 1.0;
			else
				scalex = 1.0, scaley = (double)wst->width / (double)wst->height;

			transforms[0] = f_mat_scale3d(scalex, scaley, 1.0);
		}

		struct quaternion zrotq = (struct quaternion) {
			0, 0, sin(0.5*wst->time), cos(0.5*wst->time)
		};

		// transforms[3] = f_mat_rotatez(wst->time);
		transforms[3] = f_mat_quaternion_rotate(zrotq);
		transforms[6] = f_mat_rotatez(-wst->time);
		transforms[7] = f_mat_rotatey(3*wst->time);

		const struct mat4x4f tf = f_mat_multiplylist(transforms, M_LEN(transforms));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUniformMatrix4fv(transformloc, 1, GL_TRUE, &(tf.arr[0][0]));
		glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(win);
		glfwPollEvents();
		f_render_evalstate(wst);
	}
}

unsigned int f_render_genprogram(const char* vertpath, const char* fragpath) {
	static char chbuf[CHBUFSZ_] = {0};

	const unsigned int vert = f_gl_genshader(vertpath, GL_VERTEX_SHADER, chbuf, CHBUFSZ_);
	const unsigned int frag = f_gl_genshader(fragpath, GL_FRAGMENT_SHADER, chbuf, CHBUFSZ_);
	const unsigned int sp = f_gl_genprogram(vert, frag, chbuf, CHBUFSZ_);

	glDetachShader(sp, vert);
	glDetachShader(sp, frag);

	glDeleteShader(vert);
	glDeleteShader(frag);

	return sp;
}

/* Main function wrapped around glfw initalization and window creation */
void f_render_main(void* win) {
	/* Generate and bind objects */
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	unsigned int texobj;
	glGenTextures(1, &texobj);
	glBindTexture(GL_TEXTURE_2D, texobj);

	const unsigned int sp = f_render_genprogram("shaders/vertex.glsl", "shaders/fragment.glsl");
	glUseProgram(sp);

	const int transformloc = glGetUniformLocation(sp, "transform");
	if(transformloc < 0)
		fprintf(stderr, "ERROR: Unable to get location for uniform 'transform'!\n");

	f_render_init();
	f_render_loop(win, transformloc);

	/* Cleanup */
	glDeleteTextures(1, &texobj);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteProgram(sp);
}


