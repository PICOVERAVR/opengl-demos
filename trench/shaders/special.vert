#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 vcolor;
layout (location = 3) in vec2 uv;

layout (location = 4) in mat4 model;

out vec4 color;

uniform mat4 view;
uniform mat4 projection;

uniform double offset;

out gl_PerVertex {
	vec4 gl_Position;
	//int gl_InstanceID;
};

// 1D random function from the internet, takes the very low bits of a sin function?
float rand(float n) {
	return fract(sin(n) * 43758.5453123);
}

void main() {
	gl_Position = projection * view * model * vec4(position, 1.0f);

	// pass everything in world space

	color = vcolor;
}