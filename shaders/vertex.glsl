#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 clr_in;

uniform int scale;

out vec3 clr;

void main() {
	gl_Position = vec4(pos / scale, 1.0f);
	clr = clr_in;
}

