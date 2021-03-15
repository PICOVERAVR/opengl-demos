#version 450 core

// note: when using program pipelines, all fixed pipeline functionality needs to be re-declared.

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VERTEX {
	sample vec4 vcolor;
	vec3 vnormal;
	vec2 vuv;
	vec3 vposition;
} vertex;

out gl_PerVertex {
	vec4 gl_Position; // only need this for now
};

void main() {
	gl_Position = projection * view * model * vec4(position, 1.0f);

	// pass everything in world space

	vertex.vposition = vec3(model * vec4(position, 1.0f));
	vertex.vcolor = color;
	vertex.vuv = uv;
	vertex.vnormal = normalize(mat3(model) * normal).xyz; // normalize the result since we might have done a scale or rotate or something
	// this only works if we only do uniform scaling on the object, but the alternative (inverse transpose) carries a pretty heavy compute penalty.

	// additionally, mat3 is used because since the skybox doesn't move relative to the camera, neither should the normals?
}