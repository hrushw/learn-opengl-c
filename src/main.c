#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "window.h"

void f_render_main(void* win);

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

/* Main function */
int main(void) {
	glfwSetErrorCallback(f_glfw_callback_error);
	if(!glfwInit()) return -1;

	void* const win = f_glfw_initwin("Tetrahedron", 640, 480, WIN_DEF, &ws);
	if(!win) goto end;

	f_render_main(win);

	glfwDestroyWindow(win);
	end: glfwTerminate();
	return 0;
}
