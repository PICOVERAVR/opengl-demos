#version 450 core

in VERTEX {
	sample vec4 vcolor; // color is multisampled. If variable is not 'sample'd, fragment shader executed once, with each fragment receiving the same color values (but different depth/stencil values).
	// if variable is sampled, then fragment shader will be executed per subpixel. (this is multisampling, not antialiasing.)
	vec3 vnormal;
	vec3 vuv;
	vec3 vposition;
} vertex;

out vec4 fColor;

layout (binding = 0) uniform samplerCube skybox;

void main() {
	fColor = texture(skybox, vertex.vposition);
}