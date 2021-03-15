#pragma once
#include <string>
#include <vector>
#include <cmath>

#include <thread>
#include <mutex>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>

#include "map.h"
#include "vloader.h"

enum modelnames {
	u0,
	modelsize
};

class unit {
public:
	unit() : color(0.0, 0.0, 0.0, 1.0), speed(1.0), modelmat(1.0), pos(0.0) { }
	void load(const std::string path);
	void move(std::vector<vertexinfo> const& terrainmap, glm::vec2 newpos);
	void updatepos(std::vector<vertexinfo> const& terrainmap);
	void travelto(glm::vec3 newdest) { dest = newdest; }
	void render();
	void destroy();
	
	glm::vec4 color;
	float speed;

	glm::mat4 modelmat;
private:
	glm::vec3 pos;
	glm::vec3 dest; // destination to travel towards	
	
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	size_t rendersize;
	unsigned int max_element_index;
};

