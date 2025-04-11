#version 460 core

layout(points) in;

uniform float time;

layout(triangle_strip, max_vertices = 12) out;

vec4 rotate(vec4 pos) {
	float xpos = pos.x - gl_in[0].gl_Position.x;
	float ypos = pos.y - gl_in[0].gl_Position.y;
	return vec4(
		gl_in[0].gl_Position.x + (-xpos*sin(time) + ypos*cos(time)),
		gl_in[0].gl_Position.y + ( xpos*cos(time) + ypos*sin(time)),
		0.0, 1.0
	);
}

void main() {
	gl_Position = gl_in[0].gl_Position + vec4(0.0, 0.15, 0.0, 0.0);
	gl_Position = rotate(gl_Position);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(0.0, 0.17, 0.0, 0.0);
	gl_Position = rotate(gl_Position);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(0.1, 0.09, 0.0, 0.0);
	gl_Position = rotate(gl_Position);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(0.12, 0.1, 0.0, 0.0);
	gl_Position = rotate(gl_Position);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(0.1, -0.1, 0.0, 0.0);
	gl_Position = rotate(gl_Position);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(0.12, -0.12, 0.0, 0.0);
	gl_Position = rotate(gl_Position);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(-0.1, -0.1, 0.0, 0.0);
	gl_Position = rotate(gl_Position);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(-0.12, -0.12, 0.0, 0.0);
	gl_Position = rotate(gl_Position);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.09, 0.0, 0.0);
	gl_Position = rotate(gl_Position);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(-0.12, 0.1, 0.0, 0.0);
	gl_Position = rotate(gl_Position);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(0.0, 0.15, 0.0, 0.0);
	gl_Position = rotate(gl_Position);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(0.0, 0.17, 0.0, 0.0);
	gl_Position = rotate(gl_Position);
	EmitVertex();

	EndPrimitive();
}
