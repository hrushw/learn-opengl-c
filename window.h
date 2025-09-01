#ifndef __H__WINDOW_H___
#define __H__WINDOW_H___

enum e_wintype { WIN_DEF, WIN_MAX, WIN_FSCR };

/* Different input data for key press, mouse button press, and scroll events */
struct t_glfw_inputevent_key {
	int key, action, mods;
};
struct t_glfw_inputevent_mousebutton {
	int button, action, mods;
};
struct t_glfw_inputevent_scroll {
	double sx, sy;
};

enum e_inputevent_type {
	IEV_KEYPRESS = 1,
	IEV_MOUSEBUTTON = 2,
	IEV_SCROLL = 3
};

/* Tagged union for storing multiple types of input events in a single queue */
struct t_glfw_inputevent {
	union t_glfw_inputevent_u_ {
		struct t_glfw_inputevent_key
			key_ev;
		struct t_glfw_inputevent_mousebutton
			mb_ev;
		struct t_glfw_inputevent_scroll
			scroll_ev;
	} data;
	double mx, my, time;
	enum e_inputevent_type type;
};

/* Global structure for the purpose of being modified by GLFW callback functions */
struct t_glfw_winstate {
	unsigned char szrefresh:1;
	unsigned char runstate:1;
	unsigned char iqoverflow:1;

	int width, height;

	/* Mouse x, y position */
	double mx, my;
	double time;

	unsigned int iqstart, iqlength, iqmaxsz;
	struct t_glfw_inputevent *iq;
};

void f_iqpop(struct t_glfw_inputevent *, struct t_glfw_winstate *);
int f_event_cmp_key(struct t_glfw_inputevent *, int, int, int);
void* f_glfw_initwin (
	const char*, int, int,
	enum e_wintype, struct t_glfw_winstate *
);


#endif
