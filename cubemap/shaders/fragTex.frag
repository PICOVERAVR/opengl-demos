#version 450 core

in VERTEX {
	sample vec4 vcolor; // color is multisampled. If variable is not 'sample'd, fragment shader executed once, with each fragment receiving the same color values (but different depth/stencil values).
	// if variable is sampled, then fragment shader will be executed per subpixel. (this is multisampling, not antialiasing.)
	vec3 vnormal;
	vec2 vuv;
	vec3 vposition;
} vertex;

layout (location = 0) out vec4 fColor;

//layout (binding = 0) uniform sampler2D tex; // object texture if used
layout (binding = 0) uniform samplerCube tex2; // skybox texture
uniform vec3 campos;

void main() {

	//fColor = mix(texture(tex, vertex.vuv), vertex.vcolor, 0.7); // just map the texture directly on the box

	//vec3 tc = reflect(vertex.vposition, vertex.vnormal);
	//vec3 tcr = refract(vertex.vposition, vertex.vnormal, 1.33);
	//fColor = mix(mix(texture(tex2, tcr), texture(tex2, tc), 0.4), vertex.vcolor, 0.5);
	//fColor = mix(mix(texture(tex2, tcr), texture(tex2, tc), 0.4), texture(tex, vertex.vuv), 0.5);

	vec3 vi = normalize(vertex.vposition - campos);

	vec3 tc = reflect(vi, vertex.vnormal);
	vec3 tcr = refract(vi, vertex.vnormal, 0.6528);
	fColor = mix(mix(texture(tex2, tcr), texture(tex2, tc), 0.4), vertex.vcolor, 0.5);
}
