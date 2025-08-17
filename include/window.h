#ifndef __H__WINDOW_H___
#define __H__WINDOW_H___

enum e_iqsz_ { IQSZ_ = 256 };
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

enum e_inputevent_type { IEV_KEYPRESS, IEV_MOUSEBUTTON, IEV_SCROLL };

/* Tagged union for storing multiple types of input events in a single queue */
struct t_glfw_inputevent {
	enum e_inputevent_type type;

	union t_glfw_inputevent_u_ {
		struct t_glfw_inputevent_key
			key_ev;
		struct t_glfw_inputevent_mousebutton
			mb_ev;
		struct t_glfw_inputevent_scroll
			scroll_ev;
	} ev;

	double mx, my, time;
};


/* Queue keyboard and mouse input events to be evaluated  */
struct t_glfw_inputqueue {
	unsigned int start, end;
	struct t_glfw_inputevent queue[IQSZ_];
};

/* Global structure for the purpose of being modified by GLFW callback functions */
struct t_glfw_winstate {
	unsigned char szrefresh:1;
	unsigned char runstate:1;

	int width, height;
	/* Mouse x, y position */
	double mx, my;
	double time;

	struct t_glfw_inputqueue iq;
};

#endif
