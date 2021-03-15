#version 460 core

layout (location = 0) in vec3 pos;

layout (location = 0) uniform mat4 model;
layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 proj;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	gl_Position = proj * view * model * vec4(pos, 1.0f);
}
