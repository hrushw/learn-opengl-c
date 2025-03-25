#version 460 core

layout (location=0) in vec3 aPos;
layout (location=1) in vec3 clr;
uniform float rot;

out vec3 color;

void main() {
	color = clr;
	gl_Position = vec4(aPos.x*cos(rot) + aPos.y*sin(rot), aPos.y*cos(rot) - aPos.x*sin(rot), aPos.z, 1.0);
}
