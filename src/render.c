#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

/* Keypress struct - don't care about scancode or window */
struct _glfw_inputevent {
	int key;
	int action;
	int mods;
	double mx;
	double my;
	double time;
};

enum { IQSZ_ = 64 };

struct _glfw_winstate {
	GLFWwindow* win;
	int width, height;
	const char* title;

	/* Mouse x, y position */
	double mx, my;

	/* Queue of keypresses to evaluate at once */
	struct _glfw_inputevent inputqueue[IQSZ_];
	int iqstart, iqend;
	
	double time;
};

struct _glfw_winstate ws;

/* Extensive use of global variables */
/* Singular global window, width, height, title */
GLFWwindow* win;
int win_width=640, win_height=480;
const char* win_title = "Pitch dark sky";
/* Mouse x, y position */
double mx = 0, my = 0;

/* Queue of keypresses to evaluate at once */
struct _glfw_inputevent inputqueue[IQSZ_];
int iqstart = 0, iqend = 0;

double time;

void __glfw_window_destroy(void) {
	glfwDestroyWindow(ws.win);
}

/* Immediately exit on any error encountered by GLFW */
void _glfw_callback_error(int err, const char* desc) {
	fprintf(stderr, "GLFW ERROR: %s\n(Error code - %d)\n", desc, err);
	exit(EXIT_FAILURE);
}

void inputqueuecheck(void) {
	/* Bounds check for queue just in case */
	if(iqstart < 0 || iqstart >= IQSZ_ || iqend < 0 || iqend >= 2*IQSZ_) {
		fprintf(stderr, "ERROR: Key press queue indices out of bounds!\n(start = %d, end = %d, max queue size = %d)\n", iqstart, iqend, IQSZ_);
		exit(EXIT_FAILURE);
	}

	/* iqstart must be bounded to [0, IQSZ_-1], while iqend must be bounded to [0, 2*IQSZ_-1] */
	if(iqend == iqstart + IQSZ_) {
		fprintf(stderr, "ERROR: Key press queue overflow!\n(start index = %d, max queue size = %d)\n", iqstart, IQSZ_);
		exit(EXIT_FAILURE);
	}
}

void inputqueueappend(int key, int action, int mods) {
	inputqueuecheck();

	inputqueue[iqend].key = key;
	inputqueue[iqend].action = action;
	inputqueue[iqend].mods = mods;

	/* Mouse x,y coordinates and time are not rechecked in key callback function */
	inputqueue[iqend].mx = mx;
	inputqueue[iqend].mx = my;
	inputqueue[iqend].time = time;

	iqend += 1;
	iqend %= 2*IQSZ_;
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
	mx = x;
	my = y;
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

		win_width = mode->width;
		win_height = mode->height;
	}
	if(fullscreen && windowed)
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	if(windowed)
		mon = NULL;

	ws.win = glfwCreateWindow(win_width, win_height, win_title, mon, NULL);
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
	for(int i = iqstart; i != iqend; ++i) {
		continue;
	}
	memset(inputqueue, 0, IQSZ_ * sizeof(struct _glfw_inputevent));
	iqstart = 0;
	iqend = 0;
}

/* Main function */
int main(void) {
	_glfw_initialize();
	gladLoadGL(glfwGetProcAddress);

	int frameCounter = 0;
	int run = 1;
	double t0 = glfwGetTime();
	double deltaTime = 0;
	while(!glfwWindowShouldClose(ws.win) && run) {
		/* Render */

		/* GLFW window handling */
		glfwGetFramebufferSize(ws.win, &win_width, &win_height);
		glfwSwapBuffers(ws.win);
		glfwPollEvents();

		/* Update time and other computations */
		time = glfwGetTime();
		deltaTime = time - t0;
		t0 = time;
		frameCounter += 1;

		evalqueue();

		(void)deltaTime;
	}

	exit(EXIT_SUCCESS);
}
