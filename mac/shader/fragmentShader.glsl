#version 410 core

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D texOne;
uniform sampler2D texTwo;

void main() {
	vec4 color = mix(texture(texOne, texCoord), texture(texTwo, texCoord), 0.2);
	
	FragColor = color;
}
