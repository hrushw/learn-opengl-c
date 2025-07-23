#pragma once

struct mat4x4f {
	float arr[4][4];
};

extern const struct mat4x4f c_mat4x4f_identity;
extern const struct mat4x4f c_mat4x4f_zero;

struct mat4x4f f_mat_multiply(struct mat4x4f a, struct mat4x4f b);
struct mat4x4f f_mat_multiplylist(struct mat4x4f *m, int n);

struct mat4x4f f_mat_scale3d(float x, float y, float z);
struct mat4x4f f_mat_scale(float s);

struct mat4x4f f_mat_translate(float x, float y, float z);

struct mat4x4f f_mat_rotatex(double angle);
struct mat4x4f f_mat_rotatey(double angle);
struct mat4x4f f_mat_rotatez(double angle);

struct mat4x4f f_mat_perspective(
	double fov,
	double sx, double sy,
	double near, double far
);
