#include "window.h"

void f_render_main(void* win) {
	struct t_glfw_winstate* const wst = glfwGetWindowUserPointer(win);

	for(glfwSetTime(0.0); wst->runstate; wst->time = glfwGetTime()) {
		glfwSwapBuffers(win);
		glfwPollEvents();
	}
}
