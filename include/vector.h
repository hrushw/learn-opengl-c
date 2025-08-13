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

struct mat4x4f f_mat_multiply(struct mat4x4f a, struct mat4x4f b);
struct mat4x4f f_mat_multiplylist(struct mat4x4f *m, int n);
struct mat4x4f f_mat_scale3d(float x, float y, float z);
struct mat4x4f f_mat_translate(float x, float y, float z);
struct mat4x4f f_mat_rotatex(double angle);
struct mat4x4f f_mat_rotatey(double angle);
struct mat4x4f f_mat_rotatez(double angle);
struct mat4x4f f_mat_perspective(double fov, double near, double far);

struct mat4x4f f_mat_quaternion_rotate(struct quaternion q);

static inline struct mat4x4f f_mat_scale(float s) {
	return f_mat_scale3d(s, s, s);
}

#endif
