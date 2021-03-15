#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 texcoord;

layout (location = 0) uniform mat4 model;
layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 projection;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	gl_Position = projection * view * model * vec4(position, 1.0f);
}
