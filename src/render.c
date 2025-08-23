#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <epoxy/gl.h>

#include <stdio.h>
#include <math.h>

#include "window.h"
#include "vector.h"

/* References
 * ----------

 * OpenGL Tutorials (Victor Gordan)
 * "https://www.youtube.com/playlist?list=PLPaoO-vpZnumdcb4tZc4x5Q-v7CkrQ6M-"

 * OpenGL for Beginners (OGLDEV)
 * "https://www.youtube.com/playlist?list=PLA0dXqQjCx0S04ntJKUftl6OaOgsiwHjA"

 * LearnOpenGL (Camera)
 * "https://learnopengl.com/Getting-started/Camera"

 * OpenGL 4.6 specification
 * "https://registry.khronos.org/OpenGL/specs/gl/glspec46.core.pdf"

 * GLFW documentation [window guide]
 * "https://www.glfw.org/docs/latest/window_guide.html"

 * OpenGL wiki [Rendering Pipeline Overview]
 * "https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview"
 */

#define M_LEN(x) (sizeof ((x)) / sizeof (*(x)))

enum e_filetobuf_errors {
	ERR_F2B_SUCCESS = 0,

	ERR_F2B_ZERO_SIZE_BUFFER,
	ERR_F2B_FAILED_OPEN,
	ERR_F2B_FAILED_SEEK,
	ERR_F2B_FAILED_GET_SIZE,
	ERR_F2B_BUFFER_TOO_SMALL,
	ERR_F2B_FAILED_READ,
	ERR_F2B_FAILED_CLOSE,
};

/* Read file into buffer as null-terminated string */
/* Returns 0 on success, nonzero on failure */
int f_io_filetobuf(const char* path, int* len, char* buf, unsigned int buflen) {
	if(!buflen) return ERR_F2B_ZERO_SIZE_BUFFER;

	/* Open file */
	FILE* f = fopen(path, "rb");
	long l = 0;
	int ret = ERR_F2B_SUCCESS;

	/* Attempts in order: */
	/* 1 . Check if file opened successfully */
	/* 2 . Seek to end of file */
	/* 3 . Get file size */
	/* 4 . Check if buffer is large enough to store the file */
	/* 5 . Seek to start of file and read contents into buffer */

	if(!f)
		return ERR_F2B_FAILED_OPEN;
	else if( fseek(f, 0L, SEEK_END) == -1 )
		ret = ERR_F2B_FAILED_SEEK;
	else if( (l = ftell(f)) < 0 )
		ret = ERR_F2B_FAILED_GET_SIZE;
	else if( l > buflen - 1 )
		ret = ERR_F2B_BUFFER_TOO_SMALL;
	else if( rewind(f), fread(buf, sizeof(char), l, f) != (size_t)l )
		ret = ERR_F2B_FAILED_READ;
	else {
		/* Add null terminator */
		buf[l] = 0;
		goto end;
	}

	buf[0] = 0;

	end:
	if(len) *len = l;
	if(fclose(f)) return ERR_F2B_FAILED_CLOSE;
	return ret;
}

enum e_gl_shader_check {
	ERR_GL_SHADER_SUCCESS = 0,
	ERR_GL_SHADER_FAIL,
	ERR_GL_SHADER_FAIL_LOG_INCOMPLETE,
};

/* Check if shader was compiled successfully */
int f_gl_chkcmp(unsigned int s, unsigned int *log_len, char* infolog, int il_len) {
	int ret = 0;
	glGetShaderiv(s, GL_COMPILE_STATUS, &ret);
	if(ret) return ERR_GL_SHADER_SUCCESS;

	if(!infolog) return ERR_GL_SHADER_FAIL;

	int gl_il_len;
	glGetShaderiv(s, GL_INFO_LOG_LENGTH, &gl_il_len);
	if(log_len) *log_len = gl_il_len;

	glGetShaderInfoLog(s, il_len, NULL, infolog);
	infolog[il_len-1] = 0;

	return gl_il_len > il_len ?
		ERR_GL_SHADER_FAIL_LOG_INCOMPLETE :
		ERR_GL_SHADER_FAIL ;
}

/* Check if program was linked successfully */
int f_gl_chklink(unsigned int sp, unsigned int *log_len, char* infolog, int il_len) {
	int ret = 0;
	glGetProgramiv(sp, GL_LINK_STATUS, &ret);
	if(ret) return ERR_GL_SHADER_SUCCESS;

	if(!infolog) return ERR_GL_SHADER_FAIL;

	int gl_il_len;
	glGetProgramiv(sp, GL_INFO_LOG_LENGTH, &gl_il_len);
	if(log_len) *log_len = gl_il_len;

	glGetProgramInfoLog(sp, il_len, NULL, infolog);
	infolog[il_len-1] = 0;

	return gl_il_len > il_len ?
		ERR_GL_SHADER_FAIL_LOG_INCOMPLETE :
		ERR_GL_SHADER_FAIL ;
}

int f_gl_genshader(
	unsigned int *s, int type, const char* srcbuf,
	char* logbuf, unsigned int logbufsz, unsigned int* log_len
) {
	*s = glCreateShader(type);
	glShaderSource(*s, 1, &srcbuf, NULL);
	glCompileShader(*s);

	return f_gl_chkcmp(*s, log_len, logbuf, logbufsz);
}

/* Generate shader program from given vertex and fragment shaders */
int f_gl_genprogram(
	unsigned int *sp, unsigned int vert, unsigned int frag,
	char* logbuf, unsigned int logbufsz, unsigned int* log_len
) {
	*sp = glCreateProgram();

	glAttachShader(*sp, vert);
	glAttachShader(*sp, frag);

	glLinkProgram(*sp);

	return f_gl_chklink(*sp, log_len, logbuf, logbufsz);
}


/* Evaluate keyboard and mouse events */
/* Currently handles shortcuts for resetting time and exit */
void f_render_evalstate_key(struct t_glfw_winstate *wst, struct t_glfw_inputevent *e) {
	struct t_glfw_inputevent_key *kev = &e->ev.key_ev;

	if(
		kev->key == GLFW_KEY_T &&
		kev->mods == GLFW_MOD_CONTROL &&
		kev->action == GLFW_RELEASE
	)
		glfwSetTime(0.0), wst->time = 0.0;

	else if(
		kev->key == GLFW_KEY_Q &&
		kev->mods == (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT) &&
		kev->action == GLFW_RELEASE
	)
		wst->runstate = 0;
}

void f_render_evalstate(struct t_glfw_winstate *wst) {
	for(unsigned int i = wst->iq.start; (i %= IQSZ_) != wst->iq.end; ++i) {
		struct t_glfw_inputevent *qev = &wst->iq.queue[i];

		switch(qev->type) {
		case IEV_KEYPRESS:
			f_render_evalstate_key(wst, qev);
			break;

		case IEV_SCROLL:
			// printf("SCROLLING: %.4lf %.4lf\n", qev->ev.scroll_ev.sx, qev->ev.scroll_ev.sy);
			break;

		case IEV_MOUSEBUTTON:
			break;
		}
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

		transforms[3] = f_mat_quat_rotate(f_quat_rotate(
			0, 0, 1, wst->time
		));
		transforms[6] = f_mat_rotatez(-wst->time);
		transforms[7] = f_mat_rotatey(3*wst->time);

		struct mat4x4f tf = f_mat_multiplylist(transforms, M_LEN(transforms));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUniformMatrix4fv(transformloc, 1, GL_TRUE, &(tf.arr[0][0]));
		glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(win);
		glfwPollEvents();
		f_render_evalstate(wst);
	}
}

unsigned int f_render_genprogram(const char* vertpath, const char* fragpath) {
	enum e_bufsz_shader { BUFSZ_SHADER = 0x2000 };
	enum e_bufsz_log {BUFSZ_LOG = 0x1000 };

	static char vert_srcbuf[ BUFSZ_SHADER ] = {0};
	static char frag_srcbuf[ BUFSZ_SHADER ] = {0};

	if(f_io_filetobuf(vertpath, NULL, vert_srcbuf, BUFSZ_SHADER))
		fprintf(stderr, "ERROR: Unable to get contents for file '%s'!\n", vertpath);

	if(f_io_filetobuf(fragpath, NULL, frag_srcbuf, BUFSZ_SHADER))
		fprintf(stderr, "ERROR: Unable to get contents for file '%s'!\n", fragpath);

	static char vert_logbuf[ BUFSZ_LOG ] = {0};
	static char frag_logbuf[ BUFSZ_LOG ] = {0};

	static char prog_logbuf[ BUFSZ_LOG ] = {0};

	unsigned int vert, frag, sp;

	if(f_gl_genshader (
		&vert, GL_VERTEX_SHADER, vert_srcbuf,
		vert_logbuf, BUFSZ_LOG, NULL
	)) fprintf(stderr, "ERROR: Failed to compile vertex shader! Error log:\n%s\n", vert_logbuf);

	if(f_gl_genshader (
		&frag, GL_FRAGMENT_SHADER, frag_srcbuf,
		frag_logbuf, BUFSZ_LOG, NULL
	)) fprintf(stderr, "ERROR: Failed to compile fragment shader! Error log:\n%s\n", frag_logbuf);

	if(f_gl_genprogram(&sp, vert, frag, prog_logbuf, BUFSZ_LOG, NULL))
		fprintf(stderr, "ERROR: Failed to link shader program! Error log:\n%s\n", prog_logbuf);

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

	unsigned int sp = f_render_genprogram("shaders/vertex.glsl", "shaders/fragment.glsl");
	glUseProgram(sp);

	int transformloc = glGetUniformLocation(sp, "transform");
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

