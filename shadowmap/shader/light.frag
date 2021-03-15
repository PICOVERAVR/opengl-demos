#version 460 core

layout (location = 0) uniform vec3 color;

out vec4 fColor;

void main() {
	fColor = vec4(color, 1.0f);
}
