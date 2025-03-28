#version 460 core

flat in vec3 clr;

out vec4 frag_clr;

void main() {
	frag_clr = vec4(clr, 1.0f);
}
