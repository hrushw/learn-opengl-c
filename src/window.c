#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>

#include "window.h"

void f_render_main(void* win);

/* Append to queue - bounds check merged with function */
void f_iqappend(struct t_glfw_inputqueue *q, struct t_glfw_inputevent ev) {
	/* Bounds check for queue just in case */
	/* start must be bounded to [0, IQSZ_-1], while end must be bounded to [0, 2*IQSZ_-1] */
	if(
		q->start < 0 || q->start >= IQSZ_ ||
		q->end < 0 || q->end >= 2*IQSZ_ ||
		q->end - q->start >= IQSZ_ || q->start > q->end
	) {
		/* If bounds check fails, log error and reset the queue */
		fprintf(stderr,
			"ERROR: Key press queue indices out of bounds!\n"
			"(start index = %d, end index = %d, max queue size = %d)\n",
			q->start, q->end, IQSZ_
		);
		q->start = 0, q->end = 0;
	}

	q->queue[q->end] = ev;
	q->end = (q->end + 1) % (2*IQSZ_);
}

/* ----------------------- *
 * GLFW Callback functions *
 * ----------------------- */

/* Immediately exit on any error encountered by GLFW */
void f_glfw_callback_error(int err, const char* desc) {
	fprintf(stderr, "GLFW Error: \n%s\n(Error code - %d)\n", desc, err);
}

/* Key callback: simply add pressed key to queue for evaluation, immediately exit on queue overflow */
/* Additionally store mouse coordinates into queue */
void f_glfw_callback_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
	struct t_glfw_winstate* const wst = glfwGetWindowUserPointer(window);
	f_iqappend(&wst->iq, (struct t_glfw_inputevent){ key, action, mods, wst->mx, wst->my, wst->time });

	/* Scancode remains unused */
	(void)scancode;
}

/* Cursor position callback: simply update global mouse coordinates */
void f_glfw_callback_cursorpos(GLFWwindow *window, double x, double y) {
	struct t_glfw_winstate* const wst = glfwGetWindowUserPointer(window);
	wst->mx = x, wst->my = y;
}

/* Mouse click callback: same as key callback */
/* (this assumes mouse clicks and keypresses have distinct keycodes) */
void f_glfw_callback_mouseclick(GLFWwindow *window, int button, int action, int mods) {
	struct t_glfw_winstate* const wst = glfwGetWindowUserPointer(window);
	f_iqappend(&wst->iq, (struct t_glfw_inputevent){ button, action, mods, wst->mx, wst->my, wst->time });
}

/* Callback for framebuffer resize events (i.e window resize events) */
void f_glfw_callback_fbresize(GLFWwindow *window, int width, int height) {
	struct t_glfw_winstate* const wst = glfwGetWindowUserPointer(window);
	wst->width = width, wst->height = height, wst->szrefresh = 1;
}

/* Callback for window close event */
void f_glfw_callback_winclose(GLFWwindow *window) {
	struct t_glfw_winstate* const wst = glfwGetWindowUserPointer(window);
	wst->runstate = 0;
}


/* Create window - optionally maximize and make it fullscreen */
void* f_glfw_crwin(const char* title, int width, int height, enum e_wintype type) {
	GLFWmonitor* mon = glfwGetPrimaryMonitor();
	if(!mon) return NULL;

	const GLFWvidmode* const mode = glfwGetVideoMode(mon);
	if(!mode) return NULL;

	switch(type) {
		case WIN_FSCR:
			glfwWindowHint(GLFW_RED_BITS, mode->redBits);
			glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
			glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
			glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
			break;

		case WIN_MAX:
			glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
			/* fall through */

		case WIN_DEF:
		default:
			mon = NULL;
	}

	return glfwCreateWindow(width, height, title, mon, NULL);
}

/* Initialize glfw, create window, set callback functions, initialize OpenGL context, global GLFW settings */
void* f_glfw_initwin(const char* title, int width, int height) {
	/* Initialize window with OpenGL 4.6 core context */
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, GLFW_FALSE); /* ?? */

	void* const win = f_glfw_crwin(title, width, height, WIN_DEF);
	if(!win) return NULL;

	/* Setup callback functions */
	glfwSetKeyCallback(win, f_glfw_callback_key);
	glfwSetCursorPosCallback(win, f_glfw_callback_cursorpos);
	glfwSetMouseButtonCallback(win, f_glfw_callback_mouseclick);
	glfwSetFramebufferSizeCallback(win, f_glfw_callback_fbresize);
	glfwSetWindowCloseCallback(win, f_glfw_callback_winclose);

	glfwMakeContextCurrent(win); // Setup OpenGL context
	glfwSwapInterval(1); // calls to glfwSwapBuffers() will only cause swap once per frame
	return win;
}

/* Main function */
int main(void) {
	glfwSetErrorCallback(f_glfw_callback_error);
	if(!glfwInit()) return -1;

	void* const win = f_glfw_initwin("Tetrahedron", 640, 480);
	if(!win) goto end;

	struct t_glfw_winstate ws = {
		.width = 0, .height = 0,
		.mx = 0, .my = 0,
		.time = 0,
		.iq = {
			.start = 0, .end = 0,
			.queue = {{0}}
		},
		.szrefresh = 1,
		.runstate = 1,
	};

	glfwGetFramebufferSize(win, &ws.width, &ws.height);
	glfwSetWindowUserPointer(win, &ws);

	f_render_main(win);

	glfwDestroyWindow(win);
	end: glfwTerminate();
	return 0;
}
