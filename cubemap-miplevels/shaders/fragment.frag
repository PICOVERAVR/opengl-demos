#version 450 core

in VERTEX {
	sample vec4 vcolor; // color is multisampled. If variable is not 'sample'd, fragment shader executed once, with each fragment receiving the same color values (but different depth/stencil values).
	// if variable is sampled, then fragment shader will be executed per subpixel. (this is multisampling, not antialiasing.)
	vec3 vnormal;
	vec2 vuv;
	vec3 vposition;
} vertex;

out vec4 fColor;

void main() {
	fColor = vertex.vcolor;

	// texture coordinates have to be given a transparency as well or they will behave as opaque with depth testing turned off
	//fColor = mix(vec4(texture(tex, vertex.texCoord).rrr, 1.0f), vertex.color, 0.7f); // mix = x * (1 - a) + y * a
}