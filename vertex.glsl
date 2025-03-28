#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in float vibr;
layout(location = 2) in vec3 color;
layout(location = 3) in float phase;
layout(location = 4) in float w;

out vec3 clr;

uniform float time;

void main() {
	float k = 0.5f;
	clr = color;
	gl_Position = vec4(pos.x, pos.y + k*vibr*sin(w*(phase+time)), pos.z, 1.0f);
}
