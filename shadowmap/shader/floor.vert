#version 460 core

layout (location = 0) in vec3 position;

layout (location = 0) uniform mat4 model;
layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 projection;
layout (location = 3) uniform mat4 lightView;
layout (location = 4) uniform mat4 shadow;

layout (location = 0) out vec4 scoord;

out gl_PerVertex {
	vec4 gl_Position;
};

const bool using_csm = true;

void main() {
	gl_Position = projection * view * model * vec4(position, 1.0f);
	if (using_csm) {
		scoord = lightView * model * vec4(position, 1.0f);
	} else { // use psm instead
		// we have to use a scale and bias matrix and divide by w in our FS because all that stuff 
		// happens automatically with gl_Position and we want the same NDC clip coordinates for
		// our texture call to work.
		const mat4 scaleBiasMatrix = mat4(0.5f, 0.0f, 0.0f, 0.0f,
								  0.0f, 0.5f, 0.0f, 0.0f,
								  0.0f, 0.0f, 0.5f, 0.0f,
								  0.5f, 0.5f, 0.5f, 1.0f);
		scoord = scaleBiasMatrix * shadow * lightView * model * vec4(position, 1.0f);
	}
}
