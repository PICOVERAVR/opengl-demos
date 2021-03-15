#version 460 core

layout (location = 0) in vec4 scoord;

const int num_cascades = 3;
layout (binding = 0) uniform sampler2DShadow stex[num_cascades];

layout (location = 0) uniform vec3 light_color;

layout (location = 1) uniform vec4 cascadeDepths; // assuming num_cascades <= 4
layout (location = 2) uniform mat4 shadowMatrices[num_cascades];

out vec4 fColor;

const vec3 floor_color = vec3(0.5f);

const bool using_csm = true;

const mat4 scale_bias_matrix = mat4 ( 0.5, 0.0, 0.0, 0.0,
									  0.0, 0.5, 0.0, 0.0,
									  0.0, 0.0, 0.5, 0.0,
									  0.5, 0.5, 0.5, 1.0);

void main() {
	vec4 stc_list[num_cascades];
	float pass_list[num_cascades];
	
	float pass;
	if (using_csm) {
		vec4 deeper_test = vec4(0.0);
		vec4 has_cascade;

		for (int i = 0; i < 4; i++) {
			has_cascade[i] = float(num_cascades > i);
		}
		
		for (int i = 0; i < num_cascades; i++) {
			deeper_test[i] = float(scoord.z < cascadeDepths[i]);
			stc_list[i] = scale_bias_matrix * shadowMatrices[i] * scoord;
			pass_list[i] = textureProj(stex[i], stc_list[i]);
		}
		
		float findex = clamp(dot(deeper_test, has_cascade), 0.0f, float(num_cascades - 1));
		pass = pass_list[int(findex)];
		// still using psm at the end here - not needed.
	} else {
		// might be able to get psm to work better by calculating shadow coords in frag shader, but more expensive.
		pass = textureProj(stex[0], scoord);
	}
	fColor = vec4(pass * light_color * floor_color, 1.0f);
}
