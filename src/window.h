#ifndef __H__WINDOW_H___
#define __H__WINDOW_H___

enum e_iqsz_ { IQSZ_ = 256 };
enum e_wintype { WIN_DEF, WIN_MAX, WIN_FSCR };

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

#endif
