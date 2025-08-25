#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <epoxy/gl.h>

#include <math.h>

#include "window.h"
#include "vector.h"
#include "shader.h"
#include "fileio.h"

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
	struct t_glfw_winstate* wst = glfwGetWindowUserPointer(win);

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

unsigned int f_render_genprogram(char* vertsrc, char* fragsrc) {
	enum e_bufsz_log { BUFSZ_LOG = 0x1000 };

	unsigned int vert = 0, frag = 0, sp = 0;

	if( f_gl_genshader( &vert, GL_VERTEX_SHADER, vertsrc, NULL, 0, NULL) ) goto fail_vert;

	if( f_gl_genshader( &frag, GL_FRAGMENT_SHADER, fragsrc, NULL, 0, NULL) ) goto fail_frag;

	int ret = f_gl_genprogram(&sp, vert, frag, NULL, 0, NULL);

	glDetachShader(sp, frag);
	glDetachShader(sp, vert);

	glDeleteShader(frag);

fail_frag:
	glDeleteShader(vert);

	if(ret) glDeleteProgram(sp), sp = 0;

fail_vert:
	return sp;
}

unsigned int f_render_genprogram_file(const char* vertpath, const char* fragpath) {
	enum e_bufsz_shader { BUFSZ_SHADER = 0x2000 };

	static char srcbuf[ 2 * BUFSZ_SHADER ] = {0};

	char* vert_srcbuf = srcbuf;
	char* frag_srcbuf = srcbuf + BUFSZ_SHADER;

	if(f_io_filetobuf(vertpath, NULL, vert_srcbuf, BUFSZ_SHADER)) return 0;

	if(f_io_filetobuf(fragpath, NULL, frag_srcbuf, BUFSZ_SHADER)) return 0;

	return f_render_genprogram(srcbuf, srcbuf + BUFSZ_SHADER);
}

/* Initialize OpenGL objects, start render loop, destroy objects after quit */
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

	unsigned int sp = f_render_genprogram_file("shaders/vertex.glsl", "shaders/fragment.glsl");
	glUseProgram(sp);

	int transformloc = glGetUniformLocation(sp, "transform");

	f_render_init();
	f_render_loop(win, transformloc);

	/* Cleanup */
	glDeleteTextures(1, &texobj);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteProgram(sp);
}

