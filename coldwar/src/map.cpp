#include "map.h"
using namespace std;
using namespace glm;

void map::load(vector<float> heightmap, vector<vec4> colormap, float scale) {
	int width = (int)sqrt(heightmap.size()); // heightmap is square!
	int i = 0;
	
	for (int zoffset = 0; zoffset < width - 1; zoffset++) {
		for (int xoffset = 0; xoffset < width - 1; xoffset++) {
			gputerrain.push_back({vec3(xoffset * scale, heightmap[i], zoffset * scale), colormap[i]});
			gputerrain.push_back({vec3(xoffset * scale + scale, heightmap[i + 1], zoffset * scale), colormap[i + 1]});
			gputerrain.push_back({vec3(xoffset * scale + scale, heightmap[width + i + 1], zoffset * scale + scale), colormap[width + i + 1]});
			
			gputerrain.push_back({vec3(xoffset * scale, heightmap[i], zoffset * scale), colormap[i]});
			gputerrain.push_back({vec3(xoffset * scale + scale, heightmap[width + i + 1], zoffset * scale + scale), colormap[width + i + 1]});
			gputerrain.push_back({vec3(xoffset * scale, heightmap[width + i], zoffset * scale + scale), colormap[width + i]});
			i++;
		}
		i++; // skip over the last square of each row
	}

	glCreateVertexArrays(1, &vao);
	glCreateBuffers(1, &vbo);
	glNamedBufferStorage(vbo, gputerrain.size() * sizeof(vertexinfo), gputerrain.data(), 0);
	
	glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(vertexinfo));
	glVertexArrayVertexBuffer(vao, 1, vbo, sizeof(glm::vec3), sizeof(vertexinfo));
	
	glEnableVertexArrayAttrib(vao, 0);
	glEnableVertexArrayAttrib(vao, 1);

	glVertexArrayAttribBinding(vao, 0, 0);
	glVertexArrayAttribBinding(vao, 1, 1);
	
	glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(vao, 1, 4, GL_FLOAT, GL_FALSE, 0);
	
	rendersize = gputerrain.size();
}

void map::render() {
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, rendersize);
}

void map::destroy() {
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}
