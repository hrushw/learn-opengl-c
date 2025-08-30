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

#include "vector.h"

enum e_iqsz_ { IQSZ_ = 256 };
struct t_glfw_inputevent wsqueue[IQSZ_];

unsigned int f_render_genprogram(void) {
	const char* vertpath = "shaders/vertex.glsl";
	const char* fragpath = "shaders/fragment.glsl";

	enum e_bufsz_src { BUFSZ_SRC = 0x2000 };
	enum e_bufsz_log { BUFSZ_LOG = 0x1000 };

	static char chbuf[ 2*BUFSZ_SRC + 3*BUFSZ_LOG ] = {0};

	static char* const vert_src = chbuf;
	static char* const frag_src = chbuf + BUFSZ_SRC;
	static char* const vert_log = chbuf + 2*BUFSZ_SRC;
	static char* const frag_log = chbuf + 2*BUFSZ_SRC + BUFSZ_LOG;
	static char* const prog_log = chbuf + 2*BUFSZ_SRC + 2*BUFSZ_LOG;

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

void f_iqpop(struct t_glfw_inputevent *ev, struct t_glfw_inputqueue *iq) {
	*ev = iq->queue[iq->start];
	iq->start = (iq->start + 1) % iq->maxsz;
	iq->length --;
}

int f_event_cmp_key(struct t_glfw_inputevent *ev, int key, int mods, int action) {
	if(ev->type != IEV_KEYPRESS) return 0;
	struct t_glfw_inputevent_key *k = &ev->data.key_ev;
	return (k->key == key) && (k->action == action) && (k->mods == mods);
}

void f_render_evalevent(struct t_glfw_winstate *wst, struct t_glfw_inputevent *ev) {
	if(f_event_cmp_key(ev, GLFW_KEY_Q, GLFW_MOD_CONTROL, GLFW_PRESS))
		wst->runstate = 0;
}

void f_render_main(void* win) {
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	unsigned int sp = f_render_genprogram();
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
	wst->iq.maxsz = IQSZ_;
	wst->iq.queue = wsqueue;

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
		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(*indices), GL_UNSIGNED_INT, (void*)0);
		glfwSwapBuffers(win);
		glfwPollEvents();

		for(unsigned int i = 0; i < wst->iq.length; ++i) {
			struct t_glfw_inputevent ev;
			f_iqpop(&ev, &wst->iq);
			f_render_evalevent(wst, &ev);
		}

		if(wst->iqoverflow) {
			fprintf(stderr,
				"ERROR: Key press queue indices out of bounds! logging first input...\n"
				"(start index = %d, queue length = %d, max queue size = %d)\n",
				wst->iq.start, wst->iq.length, wst->iq.maxsz
			);
			/* Pop first 10 items from queue */
			for(int i = 0; i < 10; ++i) {
				struct t_glfw_inputevent ev;
				f_iqpop(&ev, &wst->iq);
				f_log_input_type(&ev);
			}

			wst->iqoverflow = 0;
		}
	}
}
