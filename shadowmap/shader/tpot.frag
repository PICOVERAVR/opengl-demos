#version 460 core

layout (location = 0) in vec3 vn;
layout (location = 1) in vec3 vpos;

layout (location = 0) uniform vec3 light_pos;
layout (location = 1) uniform vec3 eye_pos;
layout (location = 2) uniform vec3 light_color;

out vec4 fColor;

const vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
const float shininess = 36.0f;

void main() {
	vec3 light_dir = normalize(light_pos - vpos);
	vec3 nvn = normalize(vn);
	float diffuse = dot(light_dir, nvn);
	diffuse = max(diffuse, 0.0);
	
	vec3 eye_dir = normalize(eye_pos - vpos);
	vec3 inv_light_dir = -light_dir;
	
	float spec = dot(reflect(inv_light_dir, nvn), eye_dir);
	spec = pow(max(spec, 0.0f), shininess);

	vec3 shaded_color = min((diffuse * color.rgb + spec) * light_color, vec3(1.0));
	fColor = vec4(shaded_color, color.a);
}
