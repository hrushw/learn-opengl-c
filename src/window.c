#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "window.h"

/* Append input events to queue to handle later */
void f_iqappend(struct t_glfw_winstate *wst, struct t_glfw_inputevent *ev) {
	if(wst->iqoverflow) return;
	struct t_glfw_inputqueue *q = &wst->iq;

	q->queue [(q->start + q->length) % IQSZ_] = *ev;
	q->length += 1;
	if(q->length >= IQSZ_) wst->iqoverflow = 1;

	q->start %= IQSZ_;
}

/* ----------------------- *
 * GLFW Callback functions *
 * ----------------------- */

/* Key callback: add pressed key to queue for evaluation */
/* Additionally store mouse coordinates */
void f_glfw_callback_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
	struct t_glfw_winstate* const wst = glfwGetWindowUserPointer(window);

	struct t_glfw_inputevent e = {
		IEV_KEYPRESS,
		{ .key_ev = { key, action, mods } },
		wst->mx, wst->my, wst->time
	};

	f_iqappend(wst, &e);

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

	struct t_glfw_inputevent e = {
		IEV_MOUSEBUTTON,
		{ .mb_ev = { button, action, mods } },
		wst->mx, wst->my, wst->time
	};

	f_iqappend(wst, &e);
}

/* Scroll callback: add event to queue */
void f_glfw_callback_scroll(GLFWwindow* window, double xoffset, double yoffset) {
	struct t_glfw_winstate* const wst = glfwGetWindowUserPointer(window);

	struct t_glfw_inputevent e = {
		IEV_SCROLL,
		{ .scroll_ev = { xoffset, yoffset } },
		wst->mx, wst->my, wst->time
	};

	f_iqappend(wst, &e);
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

	const GLFWvidmode* mode = glfwGetVideoMode(mon);
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

/* Create and initialize GLFW window */
void* f_glfw_initwin (
	const char* title,
	int width,
	int height,
	enum e_wintype wt,
	struct t_glfw_winstate *wst
) {
	/* Initialize window with OpenGL 4.6 core context */
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, GLFW_FALSE); /* ?? */

	void* const win = f_glfw_crwin(title, width, height, wt);
	if(!win) return NULL;

	/* Setup callback functions */
	glfwSetKeyCallback(win, f_glfw_callback_key);
	glfwSetCursorPosCallback(win, f_glfw_callback_cursorpos);
	glfwSetMouseButtonCallback(win, f_glfw_callback_mouseclick);
	glfwSetScrollCallback(win, f_glfw_callback_scroll);
	glfwSetFramebufferSizeCallback(win, f_glfw_callback_fbresize);
	glfwSetWindowCloseCallback(win, f_glfw_callback_winclose);

	/* Setup OpenGL context */
	glfwMakeContextCurrent(win);

	/* Calls to glfwSwapBuffers() will only cause swap once per frame */
	glfwSwapInterval(1);

	glfwGetFramebufferSize(win, &wst->width, &wst->height);
	glfwSetWindowUserPointer(win, wst);

	return win;
}

