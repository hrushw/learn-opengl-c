#version 460 core

layout(points) in;

layout(triangle_strip, max_vertices = 12) out;

void main() {
	gl_Position = gl_in[0].gl_Position + vec4(0.0, 0.15, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(0.0, 0.17, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(0.1, 0.09, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(0.12, 0.1, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(0.1, -0.1, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(0.12, -0.12, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(-0.1, -0.1, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(-0.12, -0.12, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.09, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(-0.12, 0.1, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(0.0, 0.15, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(0.0, 0.17, 0.0, 0.0);
	EmitVertex();
}
