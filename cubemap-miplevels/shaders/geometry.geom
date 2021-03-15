#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out; // output triangles with 3 verts

in VERTEX {
	sample vec4 vcolor;
	vec3 vnormal;
	vec2 vuv;
	vec3 vposition;
	float div;
} vertex[];

out VERTEX {
	sample vec4 vcolor;
	vec3 vnormal;
	vec2 vuv;
	vec3 vposition;
	float div;
} frag;

in gl_PerVertex {
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
} gl_in[];

out gl_PerVertex {
	vec4 gl_Position;
};

in int gl_PrimitiveIDIn;
out int gl_PrimitiveID;

void writePrimitive(int index) {
	frag.vcolor = vertex[index].vcolor;
	frag.vnormal = vertex[index].vnormal;
	frag.vuv = vertex[index].vuv;
	frag.vposition = vertex[index].vposition;
	frag.div = vertex[index].div;

	gl_Position = gl_in[index].gl_Position;
	EmitVertex();

	EndPrimitive();
}

void main() {
	writePrimitive(0);
	writePrimitive(1);
	writePrimitive(2);
}