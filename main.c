#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <epoxy/gl.h>

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include "window.h"

struct t_glfw_winstate ws = {
	.width = 0, .height = 0,
	.mx = 0, .my = 0,
	.time = 0,

	.iqstart = 0, .iqlength = 0, .iqmaxsz = 0,
	.iq = (void*)0,

	.szrefresh = 1,
	.runstate = 1,
	.iqoverflow = 0,
};

const char* vert_src =
"#version 460 core\n"
"\n"
"layout(location = 0) in vec3 pos;\n"
"layout(location = 1) in vec3 clr_in;\n"
"\n"
"out vec3 clr;\n"
"\n"
"void main() {\n"
"	gl_Position = vec4(pos, 1.0f);\n"
"	clr = clr_in;\n"
"}\n"
;

const char* frag_src =
"#version 460 core\n"
"\n"
"in vec3 clr;\n"
"\n"
"out vec4 frag_clr;\n"
"void main() {\n"
"	frag_clr = vec4(clr, 1.0f);\n"
"}\n"
;

struct vert {
	int32_t pos[3];
	uint8_t clr[3];
};

struct vert vertices[] = {
	{ { -1, -1, 0 }, { 0xFF, 0x00, 0x00 } },
	{ {  1, -1, 0 }, { 0x00, 0xFF, 0x00 } },
	{ {  0,  1, 0 }, { 0x00, 0x00, 0xFF } },
};

void f_render_main(void* win) {
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, sizeof(struct vert), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(struct vert), (void*)(offsetof(struct vert, clr)));

	unsigned int vert = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert, 1, &vert_src, NULL);
	glCompileShader(vert);

	unsigned int frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag, 1, &frag_src, NULL);
	glCompileShader(frag);

	unsigned int sp = glCreateProgram();
	glAttachShader(sp, vert);
	glAttachShader(sp, frag);
	glLinkProgram(sp);
	glUseProgram(sp);

	struct t_glfw_winstate* wst = glfwGetWindowUserPointer(win);
	for(glfwSetTime(0.0); wst->runstate; wst->time = glfwGetTime()) {
		if(wst->szrefresh)
			glViewport(0, 0, wst->width, wst->height), wst->szrefresh = 0;

		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(win);
		glfwPollEvents();
	}
}

void f_glfw_callback_error(int err, const char* desc) {
	fprintf(stderr, "GLFW Error: \n%s\n(Error code - %d)\n", desc, err);
}

/* Attempt initialization of GLFW and the window, exit if unsuccessful */
int main(void) {
	glfwSetErrorCallback(f_glfw_callback_error);
	if(!glfwInit()) return -1;

	void* win = f_glfw_initwin("[[Placeholder]]", 640, 480, WIN_MAX, &ws);
	if(!win) return glfwTerminate(), -2;

	f_render_main(win);

	glfwDestroyWindow(win);
	return glfwTerminate(), 0;
}
