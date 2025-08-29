#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <epoxy/gl.h>

#include <stdio.h>

#include "window.h"
#include "fileio.h"
#include "shader.h"
#include "errorlog.h"

unsigned int f_render_genprogram_path(const char* vertpath, const char* fragpath) {
	enum e_bufsz_src { BUFSZ_SRC = 0x2000 };
	enum e_bufsz_log { BUFSZ_LOG = 0x1000 };

	static char chbuf[ 2*BUFSZ_SRC + 3*BUFSZ_LOG ] = {0};

	static char* vert_src = chbuf;
	static char* frag_src = chbuf + BUFSZ_SRC;
	static char* vert_log = chbuf + 2*BUFSZ_SRC;
	static char* frag_log = chbuf + 2*BUFSZ_SRC + BUFSZ_LOG;
	static char* prog_log = chbuf + 2*BUFSZ_SRC + 2*BUFSZ_LOG;

	unsigned int len = 0;
	int ret = 0;

	ret = f_io_filetobuf(vertpath, &len, vert_src, BUFSZ_SRC);
	f_error_log_f2b(ret, vertpath, BUFSZ_SRC, len);

	unsigned int vert = 0;
	ret = f_gl_genshader(&vert, GL_VERTEX_SHADER, vert_src, vert_log, BUFSZ_LOG, &len);
	f_error_log_shader(ret, GL_VERTEX_SHADER, vert_log, len);

	ret = f_io_filetobuf(fragpath, &len, frag_src, BUFSZ_SRC);
	f_error_log_f2b(ret, fragpath, BUFSZ_SRC, len);

	unsigned int frag = 0;
	ret = f_gl_genshader(&frag, GL_FRAGMENT_SHADER, frag_src, frag_log, BUFSZ_LOG, &len);
	f_error_log_shader(ret, GL_FRAGMENT_SHADER, frag_log, len);

	unsigned int sp = 0;
	ret = f_gl_genprogram(&sp, vert, frag, prog_log, BUFSZ_LOG, &len);
	f_error_log_program(ret, prog_log, len);

	return sp;
}

const float vertices[] = {
	-0.6f, -0.3f, 1.0f, 0.0f, 0.0f,
	0.6f, -0.3f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.62f, 0.0f, 0.0f, 1.0f
};

void f_log_input_type(struct t_glfw_inputevent *ev) {
	switch(ev->type) {
	case IEV_KEYPRESS:
		fprintf(stderr, "Recieved keyboard input event!\n");
		break;

	case IEV_MOUSEBUTTON:
		fprintf(stderr, "Recieved mouse click input event!\n");
		break;

	case IEV_SCROLL:
		fprintf(stderr, "Recieved scroll input event!\n");
		break;

	default:
		fprintf(stderr, "ERROR: Recieved unknown input event!\n");
	}
}


void f_render_main(void* win) {
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int sp = f_render_genprogram_path("shaders/vertex.glsl", "shaders/fragment.glsl");
	glUseProgram(sp);

	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));

	struct t_glfw_winstate* wst = glfwGetWindowUserPointer(win);

	for(glfwSetTime(0.0); wst->runstate; wst->time = glfwGetTime()) {
		if(wst->szrefresh) {
			wst->szrefresh = 0;

			if(wst->width > wst->height)
				glViewport((wst->width - wst->height) / 2, 0, wst->height, wst->height);
			else
				glViewport(0, (wst->height - wst->width) / 2, wst->width, wst->width);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glfwSwapBuffers(win);
		glfwPollEvents();

		if(wst->iqoverflow) {
			fprintf(stderr,
				"ERROR: Key press queue indices out of bounds! logging first input...\n"
				"(start index = %d, queue length = %d, max queue size = %d)\n",
				wst->iq.start, wst->iq.length, IQSZ_
			);
			/* Pop first 10 items from queue */
			for(int i = 0; i < 10; ++i) {
				struct t_glfw_inputevent ev = wst->iq.queue[wst->iq.start];
				wst->iq.start = (wst->iq.start + 1) % IQSZ_;
				wst->iq.length --;

				f_log_input_type(&ev);
			}

			wst->iqoverflow = 0;
		}
	}
}
