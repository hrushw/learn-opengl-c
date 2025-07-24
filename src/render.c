#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <epoxy/gl.h>

#include <stdio.h>
#include <math.h>

#include "window.h"
#include "matrix.h"

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

unsigned int f_gl_genshader(const char* path, int type, char* chbuf, int chbufsz);

unsigned int f_gl_genprogram_vf (
	unsigned int vert,
	unsigned int frag,
	char* chbuf,
	int chbufsz
);

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

/* Evaluate keyboard and mouse events - currently handles shortcuts for resetting time and exit */
void f_render_evalstate(struct t_glfw_winstate *wst) {
	for(int i = wst->iq.start; (i %= IQSZ_) != wst->iq.end; ++i) {
		struct t_glfw_inputevent *qev = &wst->iq.queue[i];

		if (
			qev->key == GLFW_KEY_T &&
			qev->mods & GLFW_MOD_CONTROL &&
			qev->action == GLFW_RELEASE
		)
			glfwSetTime(0.0), wst->time = 0.0;

		if (
			qev->key == GLFW_KEY_Q &&
			qev->mods & GLFW_MOD_CONTROL &&
			qev->action == GLFW_RELEASE
		)
			wst->runstate = 0;
	}

	/* Reset queue after evaluation */
	wst->iq.start = 0, wst->iq.end = 0;
}

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

	/* Projection matrix must be applied last - *
	 * fixed function stage in OpenGL scales all vectors down by w */
	struct mat4x4f transforms[] = {
		c_mat4x4f_identity,
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
				scalex = (double)wst->width / (double)wst->height, scaley = 1.0;
			else
				scalex = 1.0, scaley = (double)wst->height / (double)wst->width;

			transforms[0] = f_mat_perspective(M_PI/3.0, scalex, scaley, 1.0, 6.0);
		}

		transforms[2] = f_mat_rotatez(wst->time);
		transforms[5] = f_mat_rotatez(-wst->time);
		transforms[6] = f_mat_rotatey(3*wst->time);

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
	const unsigned int sp = f_gl_genprogram_vf(vert, frag, chbuf, CHBUFSZ_);

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
	if(transformloc < 0) fprintf(stderr, "ERROR: Unable to get location for uniform 'transform'!\n");

	f_render_init();
	f_render_loop(win, transformloc);

	/* Cleanup */
	glDeleteTextures(1, &texobj);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteProgram(sp);
}

