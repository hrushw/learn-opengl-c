#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>

#include "window.h"

void f_render_main(void* win);

void* f_glfw_initwin (
	const char*, int, int,
	enum e_wintype, struct t_glfw_winstate *
);

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
	.iqoverflow = 0,
};

void f_glfw_callback_error(int err, const char* desc) {
	fprintf(stderr, "GLFW Error: \n%s\n(Error code - %d)\n", desc, err);
}

/* Attempt initialization of GLFW and the window, exit if unsuccessful */
int main(void) {
	glfwSetErrorCallback(f_glfw_callback_error);
	if(!glfwInit()) return -1;

	void* win = f_glfw_initwin("Tetrahedron", 640, 480, WIN_MAX, &ws);
	if(!win) return glfwTerminate(), -2;

	f_render_main(win);

	glfwDestroyWindow(win);
	return glfwTerminate(), 0;
}
