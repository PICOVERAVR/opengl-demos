#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

layout (location = 0) uniform mat4 model;
layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 projection;

layout (location = 0) out vec3 vn;
layout (location = 1) out vec3 vpos;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	gl_Position = projection * view * model * vec4(position, 1.0f);
	vn = vec3(mat4(mat3(model)) * vec4(normal, 1.0f));
	vpos = vec3(model * vec4(position, 1.0f));
}
