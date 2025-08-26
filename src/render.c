#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>

#include "window.h"

void f_render_main(void* win) {
	struct t_glfw_winstate* wst = glfwGetWindowUserPointer(win);

	for(glfwSetTime(0.0); wst->runstate; wst->time = glfwGetTime()) {
		glfwSwapBuffers(win);
		glfwPollEvents();
		if(wst->iqoverflow) {
			fprintf(stderr,
				"ERROR: Key press queue indices out of bounds! clearing...\n"
				"(start index = %d, end index = %d, max queue size = %d)\n",
				wst->iq.start, wst->iq.end, IQSZ_
			);

			wst->iq.start = 0;
			wst->iq.end = 0;
			wst->iqoverflow = 0;
		}
	}
}
