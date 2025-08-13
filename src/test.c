#include <stdio.h>
#include <math.h>

#include "vector.c"

void printmat(struct mat4x4f m) {
	printf(
		"[ [ %.4f, %.4f, %.4f, %.4f ]\n"
		"  [ %.4f, %.4f, %.4f, %.4f ]\n"
		"  [ %.4f, %.4f, %.4f, %.4f ]\n"
		"  [ %.4f, %.4f, %.4f, %.4f ] ]\n\n",

		m.arr[0][0], m.arr[0][1], m.arr[0][2], m.arr[0][3],
		m.arr[1][0], m.arr[1][1], m.arr[1][2], m.arr[1][3],
		m.arr[2][0], m.arr[2][1], m.arr[2][2], m.arr[2][3],
		m.arr[3][0], m.arr[3][1], m.arr[3][2], m.arr[3][3]
	);
}

int main(void) {
	double angle=0.79;
	struct quaternion zrotq = (struct quaternion) {
		0, 0, sin(angle), cos(angle)
	};

	printf("%.2f %.2f %.2f %.2f\n", zrotq.x, zrotq.y, zrotq.z, zrotq.w);

	struct mat4x4f zrotq_appl = f_mat_quaternion_apply(zrotq);
	struct mat4x4f zrotq_appl_rev = f_mat_quaternion_apply_rev(zrotq);
	struct mat4x4f zrotq_rot = f_mat_quaternion_rotate(zrotq);

	printmat(zrotq_appl);
	printmat(zrotq_appl_rev);
	printmat(zrotq_rot);
}
