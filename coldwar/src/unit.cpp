#include "unit.h"
using namespace std;
using namespace glm;

unit models[modelsize];
std::mutex modelmut;

void unit::load(const std::string path) {
	vload::vloader objloader(path);

	glCreateVertexArrays(1, &vao);
	glCreateBuffers(1, &vbo);
	glNamedBufferStorage(vbo, objloader.meshList[0].pList.size() * sizeof(vload::pt), objloader.meshList[0].pList.data(), 0);

	glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(vload::pt));
	glEnableVertexArrayAttrib(vao, 0);
	glVertexArrayAttribBinding(vao, 0, 0);
	glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	
	glVertexAttrib4f(1, 1.0, 1.0, 1.0, 1.0); // color the model white by default, can change later
	
	glCreateBuffers(1, &ebo);
	glNamedBufferStorage(ebo, objloader.meshList[0].elemList.size() * sizeof(unsigned int), objloader.meshList[0].elemList.data(), 0);
	glVertexArrayElementBuffer(vao, ebo);

	rendersize = objloader.meshList[0].elemList.size();
	max_element_index = objloader.meshList[0].elemList[0];
	for (unsigned int i = 0; i < objloader.meshList[0].elemList.size(); i++) {
		unsigned int current_index = objloader.meshList[0].elemList[i];
		if (current_index > max_element_index) {
			max_element_index = current_index;
		}
	}
}

// teleport to a location.
void unit::move(std::vector<vertexinfo> const& terrainmap, vec2 newpos) {
	pos.x = newpos.x;
	pos.z = newpos.y;

	// find the triangle that the unit is located in
	vec2 min = vec2(floor(pos.x), floor(pos.z));
	int n = 0;
	if (pos.y > pos.x) {
		n += 3;
	}
	size_t offset = 6 * min.x + 24 * min.y + n;
	
	// get triangle coordinates
	vec3 t1 = terrainmap[offset].pos;
	vec3 t2 = terrainmap[offset+1].pos;
	vec3 t3 = terrainmap[offset+2].pos;
	
	// figure out the normal of this triangle
	vec3 norm = cross(t3 - t1, t2 - t1); // assuming normals all point upwards, since we have no need to drive upside down
	
	// use a shuffled point-normal form to find required z height
	pos.y = -(norm.x * (pos.x - t1.x) + norm.z * (pos.z - t1.z) - (norm.y * t1.y)) / norm.y;
	
	modelmat = mat4(1.0); // reset model matrix

	modelmat = translate(modelmat, pos);
	modelmat = rotate(modelmat, abs(atan(norm.z / norm.y)), vec3(0.0, 0.0, 1.0));
	modelmat = rotate(modelmat, abs(atan(norm.x / norm.y)), vec3(1.0, 0.0, 0.0));
	modelmat = scale(modelmat, vec3(0.3));
}

void unit::updatepos(std::vector<vertexinfo> const& terrainmap) {
	vec2 dist = vec2(dest.x - pos.x, dest.z - pos.z);

	if (dist.x < 0.001) {
		dist.x = 0.0;
	}

	if (dist.y < 0.001) {
		dist.y = 0.0;
	}

	if (abs(dist.x) < 0.001 && abs(dist.y) < 0.001) {
		return; // if close, enough, don't move
	}
	float totaldist = sqrt(dot(dist.x, dist.y));
	float time = totaldist / speed; // time in sec
	unsigned int numframes = ceil(60.0 * time); // assumption: we're running at 60fps
	vec2 delta = vec2(dist.x / numframes, dist.y / numframes);
	this->move(terrainmap, vec2(pos.x + delta.x, pos.z + delta.y));
}

void unit::render() {
	glBindVertexArray(vao);
	// this is significantly faster than a straight drawElements on intel hardware
	glDrawRangeElements(GL_TRIANGLES, 0, max_element_index, rendersize, GL_UNSIGNED_INT, nullptr);
}

void unit::destroy() {
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);
}
