#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <epoxy/gl.h>

#include <stdio.h>
#include <math.h>

#include <stddef.h>
#include <stdint.h>

#include "window.h"
#include "fileio.h"
#include "shader.h"
#include "errorlog.h"

/* References
 * ----------

 * OpenGL Tutorials (Victor Gordan)
 * "https://www.youtube.com/playlist?list=PLPaoO-vpZnumdcb4tZc4x5Q-v7CkrQ6M-"

 * OpenGL for Beginners (OGLDEV)
 * "https://www.youtube.com/playlist?list=PLA0dXqQjCx0S04ntJKUftl6OaOgsiwHjA"
 * "https://ogldev.org/index.html"

 * LearnOpenGL
 * "https://learnopengl.com/Getting-started/Hello-Triangle"
 * "https://learnopengl.com/Getting-started/Camera"

 * OpenGL 4.6 specification
 * "https://registry.khronos.org/OpenGL/specs/gl/glspec46.core.pdf"

 * OpenGL wiki [Rendering Pipeline Overview]
 * "https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview"
 */

#define M_LEN(x) (sizeof ((x)) / sizeof (*(x)))

unsigned int f_render_genprogram(char* vert_src, char* frag_src) {
	enum e_bufsz_log { BUFSZ_LOG = 0x1000 };

	static char chbuf[ 3*BUFSZ_LOG ] = {0};

	static char* const vert_log = chbuf;
	static char* const frag_log = chbuf + BUFSZ_LOG;
	static char* const prog_log = chbuf + 2*BUFSZ_LOG;

	int ret = 0;
	unsigned int len = 0;

	unsigned int vert = 0;
	ret = f_gl_genshader(&vert, GL_VERTEX_SHADER, vert_src, vert_log, BUFSZ_LOG, &len);
	f_error_log_shader(ret, GL_VERTEX_SHADER, vert_log, len);

	unsigned int frag = 0;
	ret = f_gl_genshader(&frag, GL_FRAGMENT_SHADER, frag_src, frag_log, BUFSZ_LOG, &len);
	f_error_log_shader(ret, GL_FRAGMENT_SHADER, frag_log, len);

	unsigned int sp = 0;
	ret = f_gl_genprogram(&sp, vert, frag, prog_log, BUFSZ_LOG, &len);
	f_error_log_program(ret, prog_log, len);

	return sp;
}

unsigned int f_render_genprogram_path(const char* vertpath, const char* fragpath) {
	enum e_bufsz_src { BUFSZ_SRC = 0x2000 };
	static char chbuf[ 2*BUFSZ_SRC ] = {0};

	static char* const vert_src = chbuf;
	static char* const frag_src = chbuf + BUFSZ_SRC;

	unsigned int len_v = 0, len_f = 0;
	int ret_v = 0, ret_f = 0;

	ret_v = f_io_filetobuf(vertpath, &len_v, vert_src, BUFSZ_SRC);
	ret_f = f_io_filetobuf(fragpath, &len_f, frag_src, BUFSZ_SRC);

	f_error_log_f2b(ret_v, vertpath, BUFSZ_SRC, len_v);
	f_error_log_f2b(ret_f, fragpath, BUFSZ_SRC, len_f);

	return f_render_genprogram(vert_src, frag_src);
}

struct vert {
	int32_t pos[3];
	uint8_t rgb[3];
};

struct vert vertices[] = {
	{ {0, 0, 0}, {0x00, 0x00, 0x7F} },
	{ {1, 0, 0}, {0x00, 0x00, 0x7F} },
	{ {0, 1, 0}, {0x7F, 0x3F, 0x3F} },
	{ {1, 1, 0}, {0x3F, 0x7F, 0x3F} },

	{ {-2, -3, 0}, {0x00, 0x7F, 0x00} },
	{ {-2, -1, 1}, {0x7F, 0x3F, 0x00} },
	{ {-1, -2, 0}, {0x00, 0x7F, 0x00} },
	{ {-1, -1, 0}, {0x00, 0x7F, 0x00} },

	{ {-2,  2, 0}, {0xFF, 0x00, 0x00} },
	{ {-3,  2, 1}, {0x00, 0xFF, 0x00} },
	{ {-3,  3, 0}, {0x00, 0x00, 0xFF} },
};

uint32_t indices[] = {
	0, 1, 2,
	1, 2, 3,
	4, 5, 6,
	5, 6, 7,
	8, 9, 10
};

void f_render_main(void* win) {
	/* TODO: cleanup stuff for OpenGL objects */
	/* currently only creating one set so its fine */
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	unsigned int sp = f_render_genprogram_path("shaders/vertex.glsl", "shaders/fragment.glsl");
	glUseProgram(sp);

	int scaleloc = glGetUniformLocation(sp, "scale");
	if(scaleloc < 0) fprintf(stderr,
		"ERROR: Unable to get location for uniform 'scale'!\n"
	);
	glUniform1i(scaleloc, 4);

	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_INT, GL_FALSE, sizeof(struct vert), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(struct vert), (void*)(offsetof(struct vert, rgb)));

	struct t_glfw_winstate* wst = glfwGetWindowUserPointer(win);

	for(glfwSetTime(0.0); wst->runstate; wst->time = glfwGetTime()) {
		if(wst->szrefresh) {
			wst->szrefresh = 0;

			/*
			if(wst->width > wst->height)
				glViewport((wst->width - wst->height) / 2, 0, wst->height, wst->height);
			else
				glViewport(0, (wst->height - wst->width) / 2, wst->width, wst->width);
			*/
			glViewport(0, 0, wst->width, wst->height);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawElements(GL_TRIANGLES, M_LEN(indices), GL_UNSIGNED_INT, (void*)0);
		glfwSwapBuffers(win);
		glfwPollEvents();
	}
}

/*
TODO: some kind of moving camera shot of a 3d scene of boxes
currently the linear algebra routines in vector.h only deal with pure floats and 4x4 matrices only
they can't even apply matrices to vectors, the whole thing needs to be rewritten
*/
