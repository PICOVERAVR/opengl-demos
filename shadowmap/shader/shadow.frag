#version 460

layout (location = 0) out vec4 sColor;

void main() {
	const float fd = gl_FragCoord.z * gl_FragCoord.w;
	sColor = vec4(fd);
}
