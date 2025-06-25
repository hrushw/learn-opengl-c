#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <epoxy/gl.h>

#include <stdio.h>
#include <math.h>

/* References
 * OpenGL Tutorials (Victor Gordan) - https://www.youtube.com/playlist?list=PLPaoO-vpZnumdcb4tZc4x5Q-v7CkrQ6M-
 * OpenGL for Beginners (OGLDEV) - https://www.youtube.com/playlist?list=PLA0dXqQjCx0S04ntJKUftl6OaOgsiwHjA
 * LearnOpenGL (Camera) - https://learnopengl.com/Getting-started/Camera

 * OpenGL 4.6 specification - https://registry.khronos.org/OpenGL/specs/gl/glspec46.core.pdf
 * GLFW documentation [window guide] - https://www.glfw.org/docs/latest/window_guide.html
 * OpenGL wiki [Rendering Pipeline Overview] - https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview
 */

enum e_sizes { IQSZ_ = 256, CHBUFSZ_ = 0x10000 };

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

/* Global structure for the purpose of being modified by GLFW callback functions */
struct t_glfw_winstate {
	int width, height;
	/* Mouse x, y position */
	double mx, my;
	double time;
	struct t_glfw_inputqueue iq;
	int szrefresh;
	int runstate;
};

/* Append to queue - bounds check merged with function */
void f_iqappend(struct t_glfw_inputqueue *q, struct t_glfw_inputevent ev) {
	/* Bounds check for queue just in case */
	/* start must be bounded to [0, IQSZ_-1], while end must be bounded to [0, 2*IQSZ_-1] */
	if(q->start < 0 || q->start >= IQSZ_ || q->end < 0 || q->end >= 2*IQSZ_ || q->end - q->start >= IQSZ_ || q->start > q->end ) {
		/* If bounds check fails, log error and reset the queue */
		fprintf(stderr, "ERROR: Key press queue indices out of bounds!\n(start index = %d, end index = %d, max queue size = %d)\n", q->start, q->end, IQSZ_);
		q->start = 0, q->end = 0;
	}

	q->queue[q->end] = ev;
	q->end = (q->end + 1) % (2*IQSZ_);
}


/* Read file into buffer */
/* No longer causes program exit on failure */
/* Returns length 0 on any failure, always null terminated */
void f_io_filetobuf(const char* path, int* len, char* buf, int buflen) {
	FILE* const f = fopen(path, "rb");
	long l = 0;
	if(!f) {
		fprintf(stderr, "ERROR: Failed to open file '%s'!\n", path);
		return;
	} else if( fseek(f, 0L, SEEK_END) == -1 )
		fprintf(stderr, "ERROR: Failed to seek to end of file '%s'!\n", path);
	else if( (l = ftell(f)) < 0 )
		fprintf(stderr, "ERROR: Failed to get size of file '%s'\n", path);
	else if( l > buflen - 1 )
		fprintf(stderr, "ERROR: File '%s' too large!\n(max size = %d bytes)\n", path, buflen - 1);
	else if( rewind(f), fread(buf, sizeof(char), l, f) != (size_t)l )
		fprintf(stderr, "ERROR: Error occured while reading file '%s'!\n", path);
	else goto finish;
	/* On any read failures return empty string */
	l = 0;

	/* Always return null-terminated string */
	finish: buf[l] = 0;
	if(len) *len = l;
	if(fclose(f)) fprintf(stderr, "ERROR: Unable to close file '%s'\n", path);
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
unsigned int f_gl_genshader(const char* path, int type, char* chbuf, int chbufsz) {
	int len = 0;
	f_io_filetobuf(path, &len, chbuf, chbufsz);

	const unsigned int s = glCreateShader(type);
	glShaderSource(s, 1, (const char* const*)(&chbuf), NULL);
	glCompileShader(s);

	f_gl_chkcmp(s, chbuf, chbufsz);
	return s;
}

/* Check if program was linked successfully */
void f_gl_chklink(unsigned int sp, char* infolog, int il_len) {
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

/* Generate shader program from given vertex and fragment shaders */
unsigned int f_gl_genprogram(unsigned int vert, unsigned int frag, char* chbuf, int chbufsz) {
	unsigned int sp = glCreateProgram();

	glAttachShader(sp, vert);
	glAttachShader(sp, frag);

	glLinkProgram(sp);

	f_gl_chklink(sp, chbuf, chbufsz);
	return sp;
}

static inline void updatetime(double *time, double *t0, double *dt) {
	*time = glfwGetTime(), *dt = *time - *t0, *t0 = *time;
}

/* Evaluate keyboard and mouse events - currently handles shortcuts for hot reloading and exit */
void f_render_evalstate(struct t_glfw_winstate *wst) {
	for(int i = wst->iq.start; (i %= IQSZ_) != wst->iq.end; ++i) {
		struct t_glfw_inputevent *qev = &wst->iq.queue[i];
		// if(qev.key == GLFW_KEY_R && qev.mods == GLFW_MOD_CONTROL && qev.action == GLFW_RELEASE)
		// 	f_gl_prog_destroy(), f_gl_prog_create();
		if(qev->key == GLFW_KEY_T && qev->mods == GLFW_MOD_CONTROL && qev->action == GLFW_RELEASE)
			glfwSetTime(0.0), wst->time = 0;
		if(qev->key == GLFW_KEY_Q && qev->mods == (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT) && qev->action == GLFW_RELEASE )
			wst->runstate = 0;
	}
	/* Reset queue after evaluation */
	wst->iq.start = 0, wst->iq.end = 0;
}


/* ---------------------- *
 * Matrix transformations *
 * ---------------------- */

struct mat4x4f {
	float arr[4][4];
};

const struct mat4x4f c_mat4x4f_identity = {{
	{ 1.0, 0.0, 0.0, 0.0 },
	{ 0.0, 1.0, 0.0, 0.0 },
	{ 0.0, 0.0, 1.0, 0.0 },
	{ 0.0, 0.0, 0.0, 1.0 },
}};

const struct mat4x4f c_mat4x4f_zero = {{
	{ 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0 },
}};

struct mat4x4f f_mat_scale3d(float x, float y, float z) {
	return (struct mat4x4f){{
		{   x, 0.0, 0.0, 0.0 },
		{ 0.0,   y, 0.0, 0.0 },
		{ 0.0, 0.0,   z, 0.0 },
		{ 0.0, 0.0, 0.0, 1.0 },
	}};
}

static inline struct mat4x4f f_mat_scale(float s) {
	return f_mat_scale3d(s, s, s);
}

struct mat4x4f f_mat_translate(float x, float y, float z) {
	return (struct mat4x4f){{
		{ 1.0, 0.0, 0.0,   x },
		{ 0.0, 1.0, 0.0,   y },
		{ 0.0, 0.0, 1.0,   z },
		{ 0.0, 0.0, 0.0, 1.0 },
	}};
}

struct mat4x4f f_mat_rotatex(double angle) {
	const float c = cos(angle);
	const float s = sin(angle);
	return (struct mat4x4f) {{
		{ 1.0, 0.0, 0.0, 0.0 },
		{ 0.0,   c,  -s, 0.0 },
		{ 0.0,   s,   c, 0.0 },
		{ 0.0, 0.0, 0.0, 1.0 },
	}};
}

struct mat4x4f f_mat_rotatey(double angle) {
	const float c = cos(angle);
	const float s = sin(angle);
	return (struct mat4x4f) {{
		{   c, 0.0,   s, 0.0 },
		{ 0.0, 1.0, 0.0, 0.0 },
		{  -s, 0.0,   c, 0.0 },
		{ 0.0, 0.0, 0.0, 1.0 },
	}};
}

struct mat4x4f f_mat_rotatez(double angle) {
	const float c = cos(angle);
	const float s = sin(angle);
	return (struct mat4x4f) {{
		{   c,  -s, 0.0, 0.0 },
		{   s,   c, 0.0, 0.0 },
		{ 0.0, 0.0, 1.0, 0.0 },
		{ 0.0, 0.0, 0.0, 1.0 },
	}};
}

struct mat4x4f f_mat_perspective(double fov, double sx, double sy, double near, double far) {
	const double zscale = (near + far)/(far - near);
	const double ztrans = 2*near*far/(near - far);
	const float sp = 1.0/tanf(fov/2.0);
	return (struct mat4x4f) {{
		{ sp/sx,   0.0,    0.0,    0.0 },
		{   0.0, sp/sy,    0.0,    0.0 },
		{   0.0,   0.0, zscale, ztrans },
		{   0.0,   0.0,    1.0,    0.0 },
	}};
}

struct mat4x4f f_mat_multiply(struct mat4x4f a, struct mat4x4f b) {
	struct mat4x4f out = c_mat4x4f_zero;
	for(int i = 0; i < 4; ++i) for(int j = 0; j < 4; ++j) for(int k = 0; k < 4; ++k)
		out.arr[i][j] += a.arr[i][k] * b.arr[k][j];
	return out;
}

struct mat4x4f f_mat_multiplylist(struct mat4x4f *m, int n) {
	if(n == 0) return c_mat4x4f_identity;

	struct mat4x4f out = m[0];
	for(int i = 1; i < n; ++i)
		out = f_mat_multiply(out, m[i]);
	return out;
}

/* xyz coordinates, rgb colors, texture coordinates  */
const float vertices[] = {
	 0.0f,  0.8f,  0.0f,    1.0f, 1.0f, 1.0f,    0.0f, 0.0f,
	 0.0f, -0.2f, -0.8f,    1.0f, 0.0f, 0.0f,    0.0f, 1.0f,
	 0.7f, -0.6f,  0.4f,    0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
	-0.7f, -0.6f,  0.4f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f
};

/* Index data for rendering as triangle strip */
const unsigned int indices[] = {
	0, 1, 2, 3, 0, 1
};


/* 4x4 RGB pixel data */
const float pixels[] = {
	0.6f, 1.0f, 0.8f,    0.5f, 0.9f, 0.6f,    1.0f, 0.2f, 0.4f,    0.6f, 1.0f, 0.8f,
	0.4f, 0.9f, 1.0f,    0.8f, 0.5f, 0.5f,    0.3f, 0.3f, 1.0f,    0.8f, 0.5f, 0.5f,
	0.6f, 1.0f, 0.8f,    0.5f, 0.9f, 0.6f,    1.0f, 0.2f, 0.4f,    0.6f, 1.0f, 0.8f,
	0.4f, 0.9f, 1.0f,    0.8f, 0.5f, 0.5f,    0.3f, 0.3f, 1.0f,    0.8f, 0.5f, 0.5f,
};

unsigned int f_render_genprogram(const char* vertpath, const char* fragpath) {
	static char chbuf[CHBUFSZ_] = {0};

	const unsigned int vert = f_gl_genshader(vertpath, GL_VERTEX_SHADER, chbuf, CHBUFSZ_);
	const unsigned int frag = f_gl_genshader(fragpath, GL_FRAGMENT_SHADER, chbuf, CHBUFSZ_);
	const unsigned int sp = f_gl_genprogram(vert, frag, chbuf, CHBUFSZ_);

	glDetachShader(sp, vert);
	glDetachShader(sp, frag);

	glDeleteShader(vert);
	glDeleteShader(frag);

	return sp;
}

void f_render_init(void) {
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_DYNAMIC_DRAW);

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

	// glDepthRange(-1.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	// glDepthFunc(GL_LESS);

	/* Culling overrides the depth test ??? */
	// glEnable(GL_CULL_FACE);
	// glFrontFace(GL_CW);
	// glCullFace(GL_FRONT);
}

void f_render_loop(void* win, int transformloc) {
	struct t_glfw_winstate* const wst = glfwGetWindowUserPointer(win);

	/* Projection matrix must be applied last - fixed function stage in OpenGL scales all vectors down by w */
	struct mat4x4f transforms[] = {
		c_mat4x4f_identity,
		f_mat_translate(0.6f, 0.0f, 2.0f),
		c_mat4x4f_identity,
		f_mat_translate(0.4f, 0.1f, 0.0f),
		f_mat_scale(0.5f),
		c_mat4x4f_identity,
		c_mat4x4f_identity,
	};

	/* Initialize time and loop */
	glfwSetTime(0.0);
	for(double t0 = 0, dt = 0; wst->runstate; updatetime(&wst->time, &t0, &dt)) {
		/* Set viewport and view transform matrix */
		if(wst->szrefresh) {
			wst->szrefresh = 0;
			glViewport(0, 0, wst->width, wst->height);

			double scalex, scaley;
			if(wst->width > wst->height)
				scalex = (double)wst->width / (double)wst->height, scaley = 1.0;
			else
				scalex = 1.0, scaley = (double)wst->height / (double)wst->width;

			transforms[0] = f_mat_perspective(M_PI/3.0, scalex, scaley, 1.0, 6.0);
		}

		transforms[2] = f_mat_rotatez(wst->time);
		transforms[5] = f_mat_rotatez(-wst->time);
		transforms[6] = f_mat_rotatey(3*wst->time);

		const struct mat4x4f tf = f_mat_multiplylist(transforms, (sizeof transforms)/sizeof(struct mat4x4f));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUniformMatrix4fv(transformloc, 1, GL_TRUE, &(tf.arr[0][0]));
		glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(win);
		glfwPollEvents();
		f_render_evalstate(wst);
	}
}

/* Main function wrapped around glfw initalization and window creation */
void f_render_main(void* win) {
	const unsigned int sp = f_render_genprogram("vertex.glsl", "fragment.glsl");
	glUseProgram(sp);

	/* Generate and bind objects */
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	unsigned int texobj;
	glGenTextures(1, &texobj);
	glBindTexture(GL_TEXTURE_2D, texobj);

	const int transformloc = glGetUniformLocation(sp, "transform");
	if(transformloc < 0) fprintf(stderr, "ERROR: Unable to get location for uniform 'transform'!\n");

	f_render_init();
	f_render_loop(win, transformloc);

	/* Cleanup */
	glDeleteTextures(1, &texobj);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteProgram(sp);
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
	struct t_glfw_winstate* const wst = glfwGetWindowUserPointer(window);
	f_iqappend(&wst->iq, (struct t_glfw_inputevent){ key, action, mods, wst->mx, wst->my, wst->time });
	/* Scancode remains unused */
	(void)scancode;
}

/* Cursor position callback: simply update global mouse coordinates */
void f_glfw_callback_cursorpos(GLFWwindow *window, double x, double y) {
	struct t_glfw_winstate* const wst = glfwGetWindowUserPointer(window);
	wst->mx = x, wst->my = y;
}

/* Mouse click callback: same as key callback */
/* (this assumes mouse clicks and keypresses have distinct keycodes) */
void f_glfw_callback_mouseclick(GLFWwindow *window, int button, int action, int mods) {
	struct t_glfw_winstate* const wst = glfwGetWindowUserPointer(window);
	f_iqappend(&wst->iq, (struct t_glfw_inputevent){ button, action, mods, wst->mx, wst->my, wst->time });
}

/* Callback for framebuffer resize events (i.e window resize events) */
void f_glfw_callback_fbresize(GLFWwindow *window, int width, int height) {
	struct t_glfw_winstate* const wst = glfwGetWindowUserPointer(window);
	wst->width = width, wst->height = height, wst->szrefresh = 1;
}

/* Callback for window close event */
void f_glfw_callback_winclose(GLFWwindow *window) {
	struct t_glfw_winstate* const wst = glfwGetWindowUserPointer(window);
	wst->runstate = 0;
}


enum e_wintype { WIN_DEF, WIN_MAX, WIN_FSCR };

/* Create window - optionally maximize and make it fullscreen */
void* f_glfw_crwin(const char* title, int width, int height, enum e_wintype type) {
	GLFWmonitor* mon = glfwGetPrimaryMonitor();
	if(!mon) return NULL;

	const GLFWvidmode* const mode = glfwGetVideoMode(mon);
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

	void* const win = f_glfw_crwin(title, width, height, WIN_DEF);
	if(!win) return NULL;

	/* Setup callback functions */
	glfwSetKeyCallback(win, f_glfw_callback_key);
	glfwSetCursorPosCallback(win, f_glfw_callback_cursorpos);
	glfwSetMouseButtonCallback(win, f_glfw_callback_mouseclick);
	glfwSetFramebufferSizeCallback(win, f_glfw_callback_fbresize);
	glfwSetWindowCloseCallback(win, f_glfw_callback_winclose);

	glfwMakeContextCurrent(win); // Setup OpenGL context
	glfwSwapInterval(1); // calls to glfwSwapBuffers() will only cause swap once per frame
	return win;
}

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

	void* const win = f_glfw_initwin("Tetrahedron", 640, 480);
	if(!win) goto end;

	glfwGetFramebufferSize(win, &ws.width, &ws.height);
	glfwSetWindowUserPointer(win, &ws);

	f_render_main(win);

	glfwDestroyWindow(win);
	end: glfwTerminate();
	return 0;
}
