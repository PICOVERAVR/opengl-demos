#version 450 core

in VERTEX {
	sample vec4 vcolor; // color is multisampled. If variable is not 'sample'd, fragment shader executed once, with each fragment receiving the same color values (but different depth/stencil values).
	// if variable is sampled, then fragment shader will be executed per subpixel. (this is multisampling, not antialiasing.)
	vec3 vnormal;
	vec2 vuv;
	vec3 vposition;
	float div;
} vertex;

layout (location = 0) out vec4 fColor;

//layout (binding = 1) uniform samplerCube skybox; // skybox texture
//layout (binding = 1) uniform sampler2D tex; // object texture if used

layout (binding = 1) uniform sampler2D texHandle; // lots of texture handles

uniform vec3 campos;

void main() {
	
	//fColor = vertex.vcolor;
	//fColor = texture(tex, vertex.vuv);
	//fColor = mix(textureProjLod(tex, vec3(vertex.vuv, 1.0f), 0.0f), vertex.vcolor, 0.7);

	fColor = texture(texHandle, vertex.vuv);

	// just using the vertex normal as the source for reflection/refraction operations leads to weird results

	vec3 vi = normalize(vertex.vposition - campos);

	vec3 tc = reflect(vi, vertex.vnormal);
	vec3 tcr = refract(vi, vertex.vnormal, 1.33);
	//fColor = mix(mix(texture(tex2, tcr), texture(tex2, tc), 0.4), vertex.vcolor, 0.5);
	//fColor = mix(mix(texture(skybox, tcr), texture(skybox, tc), 0.4), texture(tex, vertex.vuv), 0.5);
}