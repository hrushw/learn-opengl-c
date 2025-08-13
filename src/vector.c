#include <math.h>
#include "vector.h"

/* ---------------------- *
 * Matrix transformations *
 * ---------------------- */

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

/* Perspective projection transformation - transform camera view frustum into OpenGL rendering box */
/* The complete transformation required for this is not linear,
 * OpenGL has an extra fixed function 'perspective division' stage
 * where all coordinates are divided by the w value.
 *
 * This function should be applied only once as the last transformation,
 * or before a scaling transformation
 */

struct mat4x4f f_mat_perspective(double fov, double near, double far) {
	const double zscale = (near + far)/(far - near);
	const double ztrans = 2*near*far/(near - far);
	const float sp = 1.0/tanf(fov/2.0);

	return (struct mat4x4f) {{
		{  sp, 0.0,    0.0,    0.0 },
		{ 0.0,  sp,    0.0,    0.0 },
		{ 0.0, 0.0, zscale, ztrans },
		{ 0.0, 0.0,    1.0,    0.0 },
	}};
}

struct quaternion f_quat_rotate(double x, double y, double z, double angle) {
	double s = sin(angle/2)/sqrt(x*x + y*y + z*z);
	return (struct quaternion) {s*x, s*y, s*z, cos(angle/2)};
}

struct quaternion f_quat_conjugate(struct quaternion q) {
	return (struct quaternion) {-q.x, -q.y, -q.z, q.w};
}

/* Matrix associated with quaternion multiplication : q*p */
struct mat4x4f f_mat_quaternion_apply(struct quaternion q) {
	return (struct mat4x4f) {{
		{  q.w, -q.z,  q.y, q.x },
		{  q.z,  q.w, -q.x, q.y },
		{ -q.y,  q.x,  q.w, q.z },
		{ -q.x, -q.y, -q.z, q.w },
	}};
}

/* Matrix associated with quaternion multiplication (in reverse) : p*q */
struct mat4x4f f_mat_quaternion_apply_rev(struct quaternion q) {
	return (struct mat4x4f) {{
		{  q.w,  q.z, -q.y, q.x },
		{ -q.z,  q.w,  q.x, q.y },
		{  q.y, -q.x,  q.w, q.z },
		{ -q.x, -q.y, -q.z, q.w },
	}};
}

/* The resultant matrix happens to be the same regardless of order of multiplication */
/* This can definitely be optimized better, for now leaving it in a more understandable form */
struct mat4x4f f_mat_quaternion_rotate(struct quaternion q) {
	return f_mat_multiply(
		f_mat_quaternion_apply(q),
		f_mat_quaternion_apply_rev(f_quat_conjugate(q))
	);
}

