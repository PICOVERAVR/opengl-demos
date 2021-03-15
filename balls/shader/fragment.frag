#version 460 core

out vec4 fColor;

layout (location = 0) uniform vec3 vel;

void main() {
	fColor = vec4(clamp(abs(vel) * 10.0, vec3(0.0), vec3(1.0)), 1.0);
}
