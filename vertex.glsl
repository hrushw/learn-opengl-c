#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 in_clr;
layout(location = 2) in vec2 texcoord;

out vec3 color;
out vec2 tex_crd;

uniform mat4 transform;

void main() {
	gl_Position = transform * vec4(pos, 1.0f);
	// gl_Position = vec4(pos, 1.0f);
	color = in_clr;
	tex_crd = texcoord;
}
