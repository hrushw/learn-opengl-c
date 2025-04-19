#version 460 core

in vec3 color;
in vec2 tex_crd;

out vec4 frag_clr;

uniform sampler2D texobj;

void main() {
	frag_clr = (0.6*texture(texobj, tex_crd) + 0.4*vec4(color, 1.0f));
	// frag_clr = vec4(color, 1.0f);
	// frag_clr = texture(texobj, tex_crd);
	// frag_clr = texture(texobj, tex_crd) * vec4(color, 1.0f);
}
