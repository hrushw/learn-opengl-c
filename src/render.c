#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

enum { IQSZ_ = 256, MAXFSZ_ = 1 << 26 };

/* Keypress struct - don't care about scancode or window */
struct _glfw_inputevent {
	int key;
	int action;
	int mods;
	double mx;
	double my;
	double time;
};

struct _glfw_inputqueue {
	int start, end;
	struct _glfw_inputevent queue[IQSZ_];
};

struct _glfw_winstate {
	GLFWwindow* win;
	int width, height;
	const char* title;

	unsigned int sp;

	/* Mouse x, y position */
	double mx, my;

	/* Queue of keypresses to evaluate at once */
	struct _glfw_inputqueue iq;
	
	double time;
	double dt;

	int runstate;
};

struct _glfw_winstate ws = {
	.win = NULL,
	.width = 640, .height = 480,
	.title = "Waves",

	.mx = 0, .my = 0,

	.iq = {
		.start = 0, .end = 0,
		.queue = {{0}}
	},
	.time = 0, .dt = 0,
	.runstate = 1
};

void _die(const char* fmt, ...);
void evalqueue(struct _glfw_inputqueue *);
void _iqcheck(struct _glfw_inputqueue *);
void _iqappend(struct _glfw_inputqueue *q, int, int, int, double, double, double);

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

void _iqcheck(struct _glfw_inputqueue *q) {
	/* Bounds check for queue just in case */
	if(q->start < 0 || q->start >= IQSZ_ || q->end < 0 || q->end >= 2*IQSZ_) _die("ERROR: Key press queue indices out of bounds!\n(start = %d, end = %d, max queue size = %d)\n", q->start, q->end, IQSZ_);

	/* iqstart must be bounded to [0, IQSZ_-1], while iqend must be bounded to [0, 2*IQSZ_-1] */
	if(q->end == q->start + IQSZ_) {
		fprintf(stderr, "ERROR: Key press queue overflow - clearing queue!\n(start index = %d, max queue size = %d)\n", q->start, IQSZ_);
		evalqueue(q);
	}
}

void _iqappend(struct _glfw_inputqueue *q, int key, int action, int mods, double mx, double my, double time) {
	_iqcheck(q);

	q->queue[q->end] = (struct _glfw_inputevent) {
		.key = key,
		.action = action,
		.mods = mods,

		/* Mouse x,y coordinates and time are not rechecked in key callback function */
		.mx = mx,
		.my = my,
		.time = time
	};

	q->end += 1;
	q->end %= 2*IQSZ_;
}


/* Immediately exit on any error encountered by GLFW */
void _glfw_callback_error(int err, const char* desc) {
	_die("GLFW ERROR: %s\n(Error code - %d)\n", desc, err);
}

/* Key callback: simply add pressed key to queue for evaluation, immediately exit on queue overflow */
/* Additionally store mouse coordinates into queue */
void _glfw_callback_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
	_iqappend(&ws.iq, key, action, mods, ws.mx, ws.my, ws.time);

	/* Window and scancode remain unused */
	(void)window;
	(void)scancode;
}

/* Cursor position callback: simply update global mouse coordinates */
void _glfw_callback_cursorpos(GLFWwindow *window, double x, double y) {
	/* Window remains unused */
	(void)window;
	ws.mx = x;
	ws.my = y;
}

/* Mouse click callback: same as key callback */
/* (this assumes mouse clicks and keypresses have distinct keycodes) */
void _glfw_callback_mouseclick(GLFWwindow *window, int button, int action, int mods) {
	_iqappend(&ws.iq, button, action, mods, ws.mx, ws.my, ws.time);

	/* Window remains unused */
	(void)window;
}

void _glfw_callback_fbresize(GLFWwindow *window, int width, int height) {
	ws.width = width;
	ws.height = height;

	(void)window;
}

/* Create window - optionally maximize and make it fullscreen */
/*( unknown what occurs at windowed = 0, fullscreen = 0 ) */
void _glfw_create_window(struct _glfw_winstate *wst, int fullscreen, int windowed) {
	/* _die() is not used here, _glfw_callback_error should already handle error messages and exit */
	GLFWmonitor* mon = glfwGetPrimaryMonitor();
	if(!mon) exit(EXIT_FAILURE);

	const GLFWvidmode* mode = glfwGetVideoMode(mon);
	if(!mode) exit(EXIT_FAILURE);

	if(fullscreen && !windowed) {
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

		wst->width = mode->width;
		wst->height = mode->height;
	}
	if(fullscreen && windowed)
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	if(windowed)
		mon = NULL;

	wst->win = glfwCreateWindow(wst->width, wst->height, wst->title, mon, NULL);
	if(!wst->win) exit(EXIT_FAILURE);
}

/* Initialize glfw, create window, set callback functions, initialize OpenGL context, global GLFW settings */
void _glfw_initialize(struct _glfw_winstate *wst) {
	glfwSetErrorCallback(_glfw_callback_error);
	glfwInit();

	atexit(glfwTerminate);

	/* Initialize window with OpenGL 4.6 core context */
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SAMPLES, GLFW_FALSE);

	_glfw_create_window(wst, 0, 1);
	atexit(__glfw_window_destroy);

	glfwSetKeyCallback(wst->win, _glfw_callback_key);
	glfwSetCursorPosCallback(wst->win, _glfw_callback_cursorpos);
	glfwSetMouseButtonCallback(wst->win, _glfw_callback_mouseclick);
	glfwSetFramebufferSizeCallback(wst->win, _glfw_callback_fbresize);

	glfwMakeContextCurrent(wst->win);
	glfwSwapInterval(1);
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

	/* check shader compilation status */
	glGetShaderiv(s, GL_COMPILE_STATUS, &success);
	if(!success) {
		int gl_il_len;
		const char* typename;
		glGetShaderiv(s, GL_INFO_LOG_LENGTH, &gl_il_len);
		if(gl_il_len > il_len) fprintf(stderr, "ERROR: Unable to get shader info log - log too large!\n(size = %d, max size = %d)", gl_il_len, il_len);
		glGetShaderInfoLog(s, il_len, NULL, infolog);
		/* get type of shader for which compilation fails */
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
		fprintf(stderr, "ERROR: Failed to compile shader of type '%s'! Error log:\n%s\n", typename, infolog);
	}

	return s;
}

void addShader(unsigned int prog, unsigned int type, const char* path) {
	enum { il_len = 4096 };
	char infolog[il_len + 1] = {0};
	int success = 0;

	unsigned int shad = genShader(path, type, infolog, il_len);
	glAttachShader(prog, shad);
	glDeleteShader(shad);
	glLinkProgram(prog);
	glDetachShader(prog, shad);

	glGetProgramiv(prog, GL_LINK_STATUS, &success);
	if(!success) {
		int gl_il_len;
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &gl_il_len);
		if(gl_il_len > il_len) fprintf(stderr, "ERROR: Unable to get shader program info log - log too large!\n(size = %d, max size = %d)", gl_il_len, il_len);
		glGetProgramInfoLog(prog, il_len, NULL, infolog);
		infolog[il_len-1] = 0;
		fprintf(stderr, "ERROR: Unable to link shader program! Error log:\n%s\n", infolog);
	}
}

void genProgram() {
	ws.sp = glCreateProgram();
	addShader(ws.sp, GL_VERTEX_SHADER, "vertex.glsl");
	addShader(ws.sp, GL_FRAGMENT_SHADER, "fragment.glsl");
	atexit(__glfw_program_delete);
}

void updatetime(double *time, double *t0, double *dt) {
	*time = glfwGetTime();
	*dt = *time - *t0;
	*t0 = *time;
}

/* Currently only clears the queue and resets indices */
void evalqueue(struct _glfw_inputqueue *q) {
	for(int i = q->start; i != q->end; ++i) {
		if(q->queue[i].key == GLFW_KEY_R && q->queue[i].mods == GLFW_MOD_CONTROL)
			genProgram();
		if(q->queue[i].key == GLFW_KEY_Q && q->queue[i].mods == (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT) )
			ws.runstate = 0;
	}
	memset(q->queue, 0, IQSZ_ * sizeof(struct _glfw_inputevent));
	q->start = 0;
	q->end = 0;
}

/* Main function */
int main(void) {
	_glfw_initialize(&ws);
	gladLoadGL(glfwGetProcAddress);

	genProgram();

	double t0 = glfwGetTime();
	while(!glfwWindowShouldClose(ws.win) && ws.runstate) {

		/* GLFW window handling */
		glfwSwapBuffers(ws.win);
		glfwPollEvents();

		updatetime(&ws.time, &t0, &ws.dt);
		evalqueue(&ws.iq);
	}

	exit(EXIT_SUCCESS);
}
