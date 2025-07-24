#include <math.h>

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


struct mat4x4f f_mat_multiply(struct mat4x4f a, struct mat4x4f b) {
	struct mat4x4f out = c_mat4x4f_zero;

	for(int i = 0; i < 4; ++i)
		for(int j = 0; j < 4; ++j)
			for(int k = 0; k < 4; ++k)
				out.arr[i][j] += a.arr[i][k] * b.arr[k][j];

	return out;
}

struct mat4x4f f_mat_multiplylist(struct mat4x4f *m, int n) {
	if(n == 0)
		return c_mat4x4f_identity;

	struct mat4x4f out = m[0];
	for(int i = 1; i < n; ++i)
		out = f_mat_multiply(out, m[i]);
	return out;
}

struct mat4x4f f_mat_scale3d(float x, float y, float z) {
	return (struct mat4x4f){{
		{   x, 0.0, 0.0, 0.0 },
		{ 0.0,   y, 0.0, 0.0 },
		{ 0.0, 0.0,   z, 0.0 },
		{ 0.0, 0.0, 0.0, 1.0 },
	}};
}

struct mat4x4f f_mat_scale(float s) {
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

struct mat4x4f f_mat_perspective (
	double fov,
	double sx, double sy,
	double near, double far
) {
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

