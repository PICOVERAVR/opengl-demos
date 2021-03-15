#version 450 core

// note: when using program pipelines, all fixed pipeline functionality needs to be re-declared.

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 uv; // not using this right now but scared to remove it

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VERTEX {
	sample vec4 vcolor;
	vec3 vnormal;
	vec3 vuv;
	vec3 vposition;
} vertex;

out gl_PerVertex {
	vec4 gl_Position; // only need this for now
};

void main() {
	vec4 pos = projection * mat4(mat3(view)) * model * vec4(position, 1.0f); // turning this into a mat3 and then into a mat4 gets rid of all of the translation stuff, meaning the 
	// skybox moves with the player but doesn't move with it.
	gl_Position = pos.xyww; // dividing everything by the z-component and then setting the depth test function to less than or equal to draws the skybox before everything else, and prevents things from falling out of it

	vertex.vposition = position;
	vertex.vcolor = color;
	vertex.vuv = vec3(0.0f); // not used in cube map
	vertex.vnormal = normalize(model * vec4(normal, 1.0f)).xyz; // normalize the result since we might have done a scale or rotate or something
	// this only works if we only do uniform scaling on the object, but the alternative (inverse transpose) carries a pretty heavy compute penalty.
}