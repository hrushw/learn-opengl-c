#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

enum { IQSZ_ = 64 };

/* Keypress struct - don't care about scancode or window */
struct _glfw_inputevent {
	int key;
	int action;
	int mods;
	double mx;
	double my;
	double time;
};

struct _glfw_winstate {
	GLFWwindow* win;
	int width, height;
	const char* title;

	/* Mouse x, y position */
	double mx, my;

	/* Queue of keypresses to evaluate at once */
	struct _glfw_inputevent iq[IQSZ_];
	int iqstart, iqend;
	
	double time;
	double dt;
};

struct _glfw_winstate ws = {
	.win = NULL,
	.width = 640, .height = 480,
	.title = "C is best",

	.mx = 0, .my = 0,

	.iqstart = 0, .iqend = 0,
	.time = 0, .dt = 0
};

void __glfw_window_destroy(void) {
	glfwDestroyWindow(ws.win);
}

/* Immediately exit on any error encountered by GLFW */
void _glfw_callback_error(int err, const char* desc) {
	fprintf(stderr, "GLFW ERROR: %s\n(Error code - %d)\n", desc, err);
	exit(EXIT_FAILURE);
}

void inputqueuecheck() {
	/* Bounds check for queue just in case */
	if(ws.iqstart < 0 || ws.iqstart >= IQSZ_ || ws.iqend < 0 || ws.iqend >= 2*IQSZ_) {
		fprintf(stderr, "ERROR: Key press queue indices out of bounds!\n(start = %d, end = %d, max queue size = %d)\n", ws.iqstart, ws.iqend, IQSZ_);
		exit(EXIT_FAILURE);
	}

	/* iqstart must be bounded to [0, IQSZ_-1], while iqend must be bounded to [0, 2*IQSZ_-1] */
	if(ws.iqend == ws.iqstart + IQSZ_) {
		fprintf(stderr, "ERROR: Key press queue overflow!\n(start index = %d, max queue size = %d)\n", ws.iqstart, IQSZ_);
		exit(EXIT_FAILURE);
	}
}

void inputqueueappend(int key, int action, int mods) {
	inputqueuecheck();

	ws.iq[ws.iqend].key = key;
	ws.iq[ws.iqend].action = action;
	ws.iq[ws.iqend].mods = mods;

	/* Mouse x,y coordinates and time are not rechecked in key callback function */
	ws.iq[ws.iqend].mx = ws.mx;
	ws.iq[ws.iqend].mx = ws.my;
	ws.iq[ws.iqend].time = ws.time;

	ws.iqend += 1;
	ws.iqend %= 2*IQSZ_;
}


/* Key callback: simply add pressed key to queue for evaluation, immediately exit on queue overflow */
/* Additionally store mouse coordinates into queue */
void _glfw_callback_key(GLFWwindow* window, int key, int scancode, int action, int mods) {
	inputqueueappend(key, action, mods);

	/* Window and scancode remain unused */
	(void)window;
	(void)scancode;
}

/* Cursor position callback: simply update global mouse coordinates */
void _glfw_callback_cursorpos(GLFWwindow* window, double x, double y) {
	/* Window remains unused */
	(void)window;
	ws.mx = x;
	ws.my = y;
}

void _glfw_callback_mouseclick(GLFWwindow* window, int button, int action, int mods) {
	inputqueueappend(button, action, mods);

	/* Window remains unused */
	(void)window;
}

/* Create window - optionally maximize and make it fullscreen */
/*( unknown what occurs at windowed = 0, fullscreen = 1 ) */
void _glfw_create_window(int fullscreen, int windowed) {
	GLFWmonitor* mon = glfwGetPrimaryMonitor();
	if(!mon) exit(EXIT_FAILURE);

	const GLFWvidmode* mode = glfwGetVideoMode(mon);
	if(!mode) exit(EXIT_FAILURE);

	if(fullscreen && !windowed) {
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

		ws.width = mode->width;
		ws.height = mode->height;
	}
	if(fullscreen && windowed)
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	if(windowed)
		mon = NULL;

	ws.win = glfwCreateWindow(ws.width, ws.height, ws.title, mon, NULL);
	if(!ws.win) exit(EXIT_FAILURE);
}

/* Initialize glfw, create window, set callback functions, initialize OpenGL context, global GLFW settings */
void _glfw_initialize(void) {
	glfwSetErrorCallback(_glfw_callback_error);
	glfwInit();

	atexit(glfwTerminate);

	/* Initialize window with OpenGL 4.6 core context */
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SAMPLES, GLFW_FALSE);

	_glfw_create_window(0, 1);
	atexit(__glfw_window_destroy);

	glfwSetKeyCallback(ws.win, _glfw_callback_key);
	glfwSetCursorPosCallback(ws.win, _glfw_callback_cursorpos);
	glfwSetMouseButtonCallback(ws.win, _glfw_callback_mouseclick);

	glfwMakeContextCurrent(ws.win);
	glfwSwapInterval(1);
}

void evalqueue(void) {
	for(int i = ws.iqstart; i != ws.iqend; ++i) {
		continue;
	}
	memset(ws.iq, 0, IQSZ_ * sizeof(struct _glfw_inputevent));
	ws.iqstart = 0;
	ws.iqend = 0;
}

/* Main function */
int main(void) {
	_glfw_initialize();
	gladLoadGL(glfwGetProcAddress);

	int frameCounter = 0;
	int run = 1;
	double t0 = glfwGetTime();
	while(!glfwWindowShouldClose(ws.win) && run) {
		/* Render */

		/* GLFW window handling */
		glfwGetFramebufferSize(ws.win, &ws.width, &ws.height);
		glfwSwapBuffers(ws.win);
		glfwPollEvents();

		/* Update time and other computations */
		ws.time = glfwGetTime();
		ws.dt = ws.time - t0;
		t0 = ws.time;
		frameCounter += 1;

		evalqueue();
	}

	exit(EXIT_SUCCESS);
}
