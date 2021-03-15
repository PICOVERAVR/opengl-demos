#version 460 core
#extension GL_ARB_bindless_texture : enable

layout (location = 0) in vec2 tc;

out vec4 fColor;

layout (bindless_sampler, binding = 0) uniform sampler2D floortex;

void main() {
	fColor = texture(floortex, tc * 4.0f);
}
