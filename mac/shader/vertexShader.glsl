#version 410 core

//basic vertex shader
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

out vec2 texCoord;

uniform mat4 model; //model matrix
uniform mat4 view; //view matrix
uniform mat4 projection; //projection matrix

void main() {
	//matrix multiplication goes backwards!
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	texCoord = aTex;
}
