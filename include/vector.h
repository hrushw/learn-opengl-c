#ifndef __H__VECTOR_H___
#define __H__VECTOR_H___

/* OpenGL floating point inputs are assumed to always be 32 bit */
/* This might not work on all machines */
struct mat4x4f {
	float arr[4][4];
};

extern const struct mat4x4f c_mat4x4f_identity;
extern const struct mat4x4f c_mat4x4f_zero;

struct quaternion {
	double x, y, z, w;
};

struct mat4x4f f_mat_multiply(struct mat4x4f, struct mat4x4f);
struct mat4x4f f_mat_multiplylist(struct mat4x4f *, int);
struct mat4x4f f_mat_scale3d(float, float, float);
struct mat4x4f f_mat_translate(float, float, float);
struct mat4x4f f_mat_rotatex(double);
struct mat4x4f f_mat_rotatey(double);
struct mat4x4f f_mat_rotatez(double);
struct mat4x4f f_mat_perspective(double, double, double);

struct quaternion f_quat_rotate(double, double, double, double);
struct mat4x4f f_mat_quat_rotate(struct quaternion);

static inline struct mat4x4f f_mat_scale(float s) {
	return f_mat_scale3d(s, s, s);
}

#endif
