#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <stdio.h>
#include <math.h>

enum e_sizes { IQSZ_ = 256, CHBUFSZ_ = 1 << 16 };

/* Keypress struct - don't care about scancode or window */
struct t_glfw_inputevent {
	int key, action, mods;
	double mx, my, time;
};

/* Queue keyboard and mouse input events to be evaluated  */
struct t_glfw_inputqueue {
	int start, end;
	struct t_glfw_inputevent queue[IQSZ_];
};

struct t_glfw_winstate {
	void* win;
	int width, height;
	/* Mouse x, y position */
	double mx, my;
	double time;
	/* Queue of keypresses to evaluate at once */
	struct t_glfw_inputqueue iq;
	int szrefresh;
	int runstate;
};

/* Window state - global structure */
struct t_glfw_winstate ws = {
	.win = NULL,
	.width = 0, .height = 0,
	.mx = 0, .my = 0,
	.time = 0,
	.szrefresh = 1,
	.runstate = 1,
	.iq = {
		.start = 0, .end = 0,
		.queue = {{0}}
	},
};

char g_charbuf[CHBUFSZ_] = {0};


/* Append to queue - bounds check merged with function */
void f_iqappend(struct t_glfw_inputqueue *q, int key, int action, int mods, double mx, double my, double time) {
	/* Bounds check for queue just in case */
	/* iqstart must be bounded to [0, IQSZ_-1], while iqend must be bounded to [0, 2*IQSZ_-1] */
	if(q->start < 0 || q->start >= IQSZ_ || q->end < 0 || q->end >= 2*IQSZ_ || q->end - q->start >= IQSZ_) {
		/* If bounds check fails, log error and reset the queue */
		fprintf(stderr, "ERROR: Key press queue indices out of bounds!\n(start index = %d, end index = %d, max queue size = %d)\n", q->start, q->end, IQSZ_);
		q->start = 0, q->end = 0;
	}

	q->queue[q->end] = (struct t_glfw_inputevent) {
		.key = key, .action = action, .mods = mods,
		/* Mouse x,y coordinates and time are not rechecked in key callback function */
		.mx = mx, .my = my, .time = time
	};
	q->end = (q->end + 1) % (2*IQSZ_);
}


/* Read file into buffer */
/* No longer causes program exit on failure */
void f_io_filetobuf(const char* path, int* len, char* buf, int buflen) {
	FILE* f = fopen(path, "rb");
	long l = 0;
	if(!f) goto failopen;
	else if( fseek(f, 0L, SEEK_END) == -1 )
		fprintf(stderr, "ERROR: Failed to seek to end of file '%s'!\n", path);
	else if( (l = ftell(f)) < 0 )
		fprintf(stderr, "ERROR: Failed to get size of file '%s'\n", path);
	else if( l > buflen - 1 )
		fprintf(stderr, "ERROR: File '%s' too large!\n(max size = %d bytes)\n", path, buflen - 1);
	else if( rewind(f), fread(buf, sizeof(char), l, f) != (size_t)l )
		fprintf(stderr, "ERROR: Error occured while reading file '%s'!\n", path);

	/* Always return null-terminated string */
	buf[l] = 0;
	if(len) *len = l;
	if(fclose(f)) fprintf(stderr, "ERROR: Unable to close file '%s'\n", path);
	return;

	failopen: fprintf(stderr, "ERROR: Failed to open file '%s'!\n", path);
}

/* Check if shader was compiled successfully */
void f_gl_chkcmp(unsigned int s, char* infolog, int il_len) {
	int success = 0;
	glGetShaderiv(s, GL_COMPILE_STATUS, &success);
	if(success) return;

	int gl_il_len;
	glGetShaderiv(s, GL_INFO_LOG_LENGTH, &gl_il_len);
	if(gl_il_len > il_len)
		fprintf(stderr, "ERROR: Unable to get complete shader info log - log too large!\n(size = %d, max size = %d)\n", gl_il_len, il_len);
	glGetShaderInfoLog(s, il_len, NULL, infolog);
	infolog[il_len-1] = 0;
	fprintf(stderr, "ERROR: Failed to compile shader! Error log:\n%s\n", infolog);
}

/* Generate shader from file path - general function */
unsigned int f_gl_genshader(const char* path, int type, char* charbuf, int charbufsz) {
	unsigned int s = glCreateShader(type);
	int len = 0;
	f_io_filetobuf(path, &len, charbuf, charbufsz);
	glShaderSource(s, 1, (const char* const*)(&charbuf), NULL);;
	glCompileShader(s);
	f_gl_chkcmp(s, charbuf, charbufsz);
	return s;
}

/* Check if program was linked successfully */
void f_gl_chklink(unsigned int sp, char *infolog, int il_len) {
	int success = 0;
	glGetProgramiv(sp, GL_LINK_STATUS, &success);
	if(success) return;

	int gl_il_len;
	glGetProgramiv(sp, GL_INFO_LOG_LENGTH, &gl_il_len);
	if(gl_il_len > il_len)
		fprintf(stderr, "ERROR: Unable to get complete shader program info log - log too large!\n(size = %d, max size = %d)\n", gl_il_len, il_len);
	glGetProgramInfoLog(sp, il_len, NULL, infolog);
	infolog[il_len-1] = 0;
	fprintf(stderr, "ERROR: Unable to link shader program! Error log:\n%s\n", infolog);
}

/* Get all shaders attached to a program, detach them and delete them */
void f_gl_detachprogshaders(unsigned int sp) {
	int n = 0;
	unsigned int s;
	while(glGetProgramiv(sp, GL_ATTACHED_SHADERS, &n), n)
		glGetAttachedShaders(sp, 1, NULL, &s), glDetachShader(sp, s);
}

unsigned int f_gl_genprogram(char* infolog, int il_len, unsigned int vert, unsigned int frag) {
	unsigned int sp = glCreateProgram();

	glAttachShader(sp, vert);
	glAttachShader(sp, frag);

	glLinkProgram(sp);
	f_gl_chklink(sp, infolog, il_len);
	return sp;
}

/* Wrappers to use global buffer */

static inline unsigned int f_gl_genshader_g(const char* path, int type) {
	return f_gl_genshader(path, type, g_charbuf, CHBUFSZ_);
}

static inline unsigned int f_gl_genprogram_g(unsigned int vert, unsigned int frag) {
	return f_gl_genprogram(g_charbuf, CHBUFSZ_, vert, frag);
}


enum e_proghandlemethod { PROG_GEN, PROG_DEL, PROG_USE };

int proghandler(enum e_proghandlemethod method) {
	static unsigned int sp = 0;
	static unsigned int vert = 0;
	static unsigned int frag = 0;

	switch(method) {
		case PROG_GEN:
		case PROG_DEL:
			glDeleteProgram(sp);
			glDeleteShader(vert), glDeleteShader(frag);
			break;
		default: ;
	}

	switch(method) {
		case PROG_GEN:
			vert = f_gl_genshader_g("vertex.glsl", GL_VERTEX_SHADER);
			frag = f_gl_genshader_g("fragment.glsl", GL_FRAGMENT_SHADER);
			sp = f_gl_genprogram_g(vert, frag);
			f_gl_detachprogshaders(sp);
			/* fall through */
		case PROG_USE:
			glUseProgram(sp);
		default: ;
	}

	return 0;
}

static inline void updatetime(double *time, double *t0, double *dt) {
	*time = glfwGetTime(), *dt = *time - *t0, *t0 = *time;
}


/* Evaluate keyboard and mouse events - currently handles shortcuts for hot reloading and exit */
void evalqueue(struct t_glfw_inputqueue *q) {
	for(int i = q->start; (i %= IQSZ_) != q->end; ++i) {
		if(q->queue[i].key == GLFW_KEY_R && q->queue[i].mods == GLFW_MOD_CONTROL && q->queue[i].action == GLFW_RELEASE)
			proghandler(PROG_GEN);
		if(q->queue[i].key == GLFW_KEY_T && q->queue[i].mods == GLFW_MOD_CONTROL && q->queue[i].action == GLFW_RELEASE)
			glfwSetTime(0.0), ws.time = 0;
		if(q->queue[i].key == GLFW_KEY_Q && q->queue[i].mods == (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT) && q->queue[i].action == GLFW_RELEASE )
			ws.runstate = 0;
	}
	/* Reset queue after evaluation */
	q->start = 0, q->end = 0;
}

void f_gl_viewportfitcenter(int width, int height) {
	int pad = width > height ? (width - height) / 2 : (height - width) / 2;
	width > height ? glViewport(pad, 0, height, height) : glViewport(0, pad, width, width);
}

/* xyz coordinates, rgb colors, texture coordinates  */
float vertices[] = {
	 0.0f,  0.8f,  0.0f,    1.0f, 1.0f, 1.0f,    0.0f, 0.0f,
	 0.0f, -0.2f, -0.8f,    1.0f, 0.0f, 0.0f,    0.0f, 2.0f,
	 0.7f, -0.6f,  0.4f,    0.0f, 1.0f, 0.0f,    2.0f, 0.0f,
	-0.7f, -0.6f,  0.4f,    0.0f, 0.0f, 1.0f,    2.0f, 2.0f
};

/* Index data for rendering as triangle strip */
unsigned int indices[] = {
	0, 1, 2, 3, 0, 1
};

/* 4x4 RGB pixel data */
float pixels[] = {
	0.6f, 1.0f, 0.8f,    0.5f, 0.9f, 0.6f,    1.0f, 0.2f, 0.4f,    0.6f, 1.0f, 0.8f,
	0.4f, 0.9f, 1.0f,    0.8f, 0.5f, 0.5f,    0.3f, 0.3f, 1.0f,    0.8f, 0.5f, 0.5f,
	0.6f, 1.0f, 0.8f,    0.5f, 0.9f, 0.6f,    1.0f, 0.2f, 0.4f,    0.6f, 1.0f, 0.8f,
	0.4f, 0.9f, 1.0f,    0.8f, 0.5f, 0.5f,    0.3f, 0.3f, 1.0f,    0.8f, 0.5f, 0.5f,
};

void rotate3dfx(float pos[3], float out[3], double angle) {
	out[0] = pos[0];
	out[1] = pos[1]*cos(angle) + pos[2]*sin(angle);
	out[2] = - pos[1]*sin(angle) + pos[2]*cos(angle);
}

void rotate3dfy(float pos[3], float out[3], double angle) {
	out[1] = pos[1];
	out[2] = pos[2]*cos(angle) + pos[0]*sin(angle);
	out[0] = - pos[2]*sin(angle) + pos[0]*cos(angle);
}

void rotate3dfz(float pos[3], float out[3], double angle) {
	out[2] = pos[2];
	out[0] = pos[0]*cos(angle) + pos[1]*sin(angle);
	out[1] = - pos[0]*sin(angle) + pos[1]*cos(angle);
}

void rotate3df(float pos[3], float out[3], double anglex, double angley, double anglez) {
	float tmp[3] = {0};
	rotate3dfx(pos, out, anglex);
	rotate3dfy(out, tmp, angley);
	rotate3dfz(tmp, out, anglez);
}

/* Main function wrapped around glfw initalization and window creation */
void f_glfw_main() {
	proghandler(PROG_GEN);

	/* Vertex Array Object */
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	/* Vertex buffer object */
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	float* vrot = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

	unsigned int texobj;
	glGenTextures(1, &texobj);
	glBindTexture(GL_TEXTURE_2D, texobj);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_FLOAT, pixels);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));

	/* Enable right handed (sensible) z axis when rendering */
	glDepthRange(1, 0);
	glEnable(GL_DEPTH_TEST);

	/* Culling overrides the depth test ??? */
	// glEnable(GL_CULL_FACE);
	// glCullFace(GL_BACK);

	/* Initialize time and loop */
	double t0 = 0, dt = 0;
	glfwSetTime(0);
	do {
		/* Set viewport and clear screen before drawing */
		ws.szrefresh ? f_gl_viewportfitcenter(ws.width, ws.height), ws.szrefresh = 0 : 0;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);

		/* GLFW window handling */
		glfwSwapBuffers(ws.win);
		glfwPollEvents();

		/* Other computations */
		updatetime(&ws.time, &t0, &dt);
		evalqueue(&ws.iq);

		/* Rotate */
		for(int i = 0; i < 4; ++i)
			rotate3df(vertices + 8*i, vrot + 8*i, ws.time, 0.8*ws.time, 0);
	} while(ws.runstate);

	/* Cleanup */
	proghandler(PROG_DEL);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glDeleteTextures(1, &texobj);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
}



/* ----------------------- *
 * GLFW Callback functions *
 * ----------------------- */

/* Immediately exit on any error encountered by GLFW */
void f_glfw_callback_error(int err, const char* desc) {
	fprintf(stderr, "GLFW Error: \n%s\n(Error code - %d)\n", desc, err);
}

/* Key callback: simply add pressed key to queue for evaluation, immediately exit on queue overflow */
/* Additionally store mouse coordinates into queue */
void f_glfw_callback_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
	f_iqappend(&ws.iq, key, action, mods, ws.mx, ws.my, ws.time);
	/* Window and scancode remain unused */
	(void)window, (void)scancode;
}

/* Cursor position callback: simply update global mouse coordinates */
void f_glfw_callback_cursorpos(GLFWwindow *window, double x, double y) {
	ws.mx = x, ws.my = y;
	/* Window remains unused */
	(void)window;
}

/* Mouse click callback: same as key callback */
/* (this assumes mouse clicks and keypresses have distinct keycodes) */
void f_glfw_callback_mouseclick(GLFWwindow *window, int button, int action, int mods) {
	f_iqappend(&ws.iq, button, action, mods, ws.mx, ws.my, ws.time);
	/* Window remains unused */
	(void)window;
}

/* Callback for framebuffer resize events (i.e window resize events) */
void f_glfw_callback_fbresize(GLFWwindow *window, int width, int height) {
	ws.width = width, ws.height = height, ws.szrefresh = 1;
	/* Window remains unused */
	(void)window;
}

/* Callback for window close event */
void f_glfw_callback_winclose(GLFWwindow *window) {
	ws.runstate = 0;
	(void)window;
}


enum e_wintype { WIN_DEF, WIN_MAX, WIN_FSCR };

/* Create window - optionally maximize and make it fullscreen */
void* f_glfw_crwin(const char* title, int width, int height, enum e_wintype type) {
	GLFWmonitor* mon = glfwGetPrimaryMonitor();
	if(!mon) return NULL;

	const GLFWvidmode* mode = glfwGetVideoMode(mon);
	if(!mode) return NULL;

	switch(type) {
		case WIN_FSCR:
			glfwWindowHint(GLFW_RED_BITS, mode->redBits);
			glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
			glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
			glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
			break;
		case WIN_MAX:
			glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
			/* fall through */
		case WIN_DEF:
		default:
			mon = NULL;
	} return glfwCreateWindow(width, height, title, mon, NULL);
}

/* Initialize glfw, create window, set callback functions, initialize OpenGL context, global GLFW settings */
void* f_glfw_initwin(const char* title, int width, int height) {
	/* Initialize window with OpenGL 4.6 core context */
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, GLFW_FALSE); /* ?? */

	void* win = f_glfw_crwin(title, width, height, WIN_DEF);
	if(!win) return NULL;

	glfwSetKeyCallback(win, f_glfw_callback_key);
	glfwSetCursorPosCallback(win, f_glfw_callback_cursorpos);
	glfwSetMouseButtonCallback(win, f_glfw_callback_mouseclick);
	glfwSetFramebufferSizeCallback(win, f_glfw_callback_fbresize);
	glfwSetWindowCloseCallback(win, f_glfw_callback_winclose);

	glfwMakeContextCurrent(win);
	gladLoadGL(glfwGetProcAddress);
	glfwSwapInterval(1);
	return win;
}

/* Main function */
int main(void) {
	glfwSetErrorCallback(f_glfw_callback_error);
	if(!glfwInit()) return -1;
	if(!(ws.win = f_glfw_initwin("Tetrahedron", 640, 480))) goto end;
	glfwGetFramebufferSize(ws.win, &ws.width, &ws.height);

	f_glfw_main();

	glfwDestroyWindow(ws.win);
	end: glfwTerminate();
	return 0;
}
