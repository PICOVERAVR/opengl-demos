#pragma once
#include <string>
#include <vector>
#include <cmath>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glad/glad.h>

struct vertexinfo {
	glm::vec3 pos;
	glm::vec4 color;
};

class map {
public:
	map() : gputerrain() { }
	void load(std::vector<float> heightmap, std::vector<glm::vec4> colormap, float scale);
	void render();
	void destroy();
	std::vector<vertexinfo> gputerrain;
private:
	GLuint vao;
	GLuint vbo;
	size_t rendersize;
};

