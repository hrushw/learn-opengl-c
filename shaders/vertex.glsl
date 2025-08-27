#version 460 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec3 clr_in;

out vec3 clr;

void main() {
	gl_Position = vec4(pos, 0.0f, 1.0f);
	clr = clr_in;
}

