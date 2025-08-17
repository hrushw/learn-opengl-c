#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "window.h"

void f_render_main(void* win);

void f_glfw_callback_error(int, const char*);

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
};

/* Attempt initialization of GLFW and the window, exit if unsuccessful */
int main(void) {
	int status = 0;

	glfwSetErrorCallback(f_glfw_callback_error);
	if(!glfwInit()) return -1;

	void* win = f_glfw_initwin("Tetrahedron", 640, 480, WIN_DEF, &ws);
	if(!win) {
		status = -2;
		goto end;
	}

	f_render_main(win);

	glfwDestroyWindow(win);

	end: glfwTerminate();
	return status;
}
