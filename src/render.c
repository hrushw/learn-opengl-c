#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>

enum { IQSZ_ = 64, MAXFSZ_ = 1 << 26 };

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

	unsigned int sp;

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

	.iq = {{0, 0, 0, 0, 0, 0}},
	.iqstart = 0, .iqend = 0,
	.time = 0, .dt = 0
};

void __glfw_window_destroy(void) {
	glfwDestroyWindow(ws.win);
}

void __glfw_program_delete(void) {
	glDeleteProgram(ws.sp);
}

void _die(const char* fmt, ...) {
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	exit(EXIT_FAILURE);
}

/* Immediately exit on any error encountered by GLFW */
void _glfw_callback_error(int err, const char* desc) {
	_die("GLFW ERROR: %s\n(Error code - %d)\n", desc, err);
}

void _iqcheck() {
	/* Bounds check for queue just in case */
	if(ws.iqstart < 0 || ws.iqstart >= IQSZ_ || ws.iqend < 0 || ws.iqend >= 2*IQSZ_) _die("ERROR: Key press queue indices out of bounds!\n(start = %d, end = %d, max queue size = %d)\n", ws.iqstart, ws.iqend, IQSZ_);

	/* iqstart must be bounded to [0, IQSZ_-1], while iqend must be bounded to [0, 2*IQSZ_-1] */
	if(ws.iqend == ws.iqstart + IQSZ_) _die("ERROR: Key press queue overflow!\n(start index = %d, max queue size = %d)\n", ws.iqstart, IQSZ_);
}

void inputqueueappend(int key, int action, int mods) {
	_iqcheck();

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

/* Mouse click callback: same as key callback */
/* (this assumes mouse clicks and keypresses have distinct keycodes) */
void _glfw_callback_mouseclick(GLFWwindow* window, int button, int action, int mods) {
	inputqueueappend(button, action, mods);

	/* Window remains unused */
	(void)window;
}

/* Create window - optionally maximize and make it fullscreen */
/*( unknown what occurs at windowed = 0, fullscreen = 0 ) */
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

/* Currently only clears the queue and resets indices */
void evalqueue(void) {
	for(int i = ws.iqstart; i != ws.iqend; ++i) {
		continue;
	}
	memset(ws.iq, 0, IQSZ_ * sizeof(struct _glfw_inputevent));
	ws.iqstart = 0;
	ws.iqend = 0;
}

/* Function to read file contents into dynamically allocated buffer with too many checks */
char* filetobuf(const char* path, int* len) {
	FILE* f = fopen(path, "rb");
	if(!f) _die("ERROR: Failed to open file '%s'!\n", path);
	if(fseek(f, 0L, SEEK_END) == -1) _die("ERROR: Failed to seek to end of file '%s'!\n", path);
	long l = ftell(f);
	if(l < 0) _die("ERROR: Failed to get size of file '%s'\n", path);
	if(l > MAXFSZ_ - 1) _die("ERROR: File '%s' too large!\n(max size = %d bytes)\n", path, MAXFSZ_ - 1);

	rewind(f);

	char* buf = malloc((l + 1)*sizeof(char));
	if(!buf) _die("ERROR: Unable to allocate memory of size '%d' bytes!\n", l+1);
	if(fread(buf, sizeof(char), l, f) != (size_t)l) _die("ERROR: Error occured while reading file '%s'!\n", path);
	buf[l] = 0;

	if(fclose(f)) _die("ERROR: Unable to close file '%s'\n", path);

	if(len) *len = l;
	return buf;
}

unsigned int genShader(const char* path, GLenum type, char* infolog, int il_len) {
	char* src = filetobuf(path, NULL);
	unsigned int s = glCreateShader(type);
	int success = 0;

	glShaderSource(s, 1, (const char**)&src, NULL);;
	free(src);

	glCompileShader(s);
	glGetShaderiv(s, GL_COMPILE_STATUS, &success);
	if(!success) {
		int gl_il_len;
		const char* typename;
		glGetShaderiv(s, GL_INFO_LOG_LENGTH, &gl_il_len);
		if(gl_il_len > il_len) _die("ERROR: Unable to get shader info log - log too large!\n(size = %d, max size = %d)", gl_il_len, il_len);
		glGetShaderInfoLog(s, il_len, NULL, infolog);
		switch(type) {
			case GL_VERTEX_SHADER:
				typename = "GL_VERTEX_SHADER";
				break;
			case GL_FRAGMENT_SHADER:
				typename = "GL_FRAGMENT_SHADER";
				break;
			default:
				typename = "<unknown>";
		}
		infolog[il_len-1] = 0;
		_die("ERROR: Failed to compile shader of type '%s'! Error log:\n%s\n", typename, infolog);
	}


	return s;
}

/* Generate the shader program */
unsigned int genProgram(const char* vertpath, const char* fragpath) {
	enum { il_len = 1023 };
	int success = 0;
	char infolog[il_len + 1] = {0};

	unsigned int vert = genShader(vertpath, GL_VERTEX_SHADER, infolog, il_len);
	unsigned int frag = genShader(fragpath, GL_FRAGMENT_SHADER, infolog, il_len);

	unsigned int sp = glCreateProgram();
	glAttachShader(sp, vert);
	glAttachShader(sp, frag);

	glLinkProgram(sp);

	glDeleteShader(vert);
	glDeleteShader(frag);

	glGetProgramiv(sp, GL_LINK_STATUS, &success);
	if(!success) {
		int gl_il_len;
		glGetProgramiv(sp, GL_INFO_LOG_LENGTH, &gl_il_len);
		if(gl_il_len > il_len) _die("ERROR: Unable to get shader program info log - log too large!\n(size = %d, max size = %d)", gl_il_len, il_len);
		glGetProgramInfoLog(sp, il_len, NULL, infolog);
		infolog[il_len-1] = 0;
		_die("ERROR: Unable to link shader program! Error log:\n%s\n", infolog);
	}
	return sp;
}

/* Main function */
int main(void) {
	_glfw_initialize();
	gladLoadGL(glfwGetProcAddress);

	ws.sp = genProgram("vertex.glsl", "fragment.glsl");
	atexit(__glfw_program_delete);

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
