#pragma once

#include "window.h"

#include "glm/glm.hpp"
#include <cmath>

class camera {
public:
	glm::vec3 pos;
	glm::vec3 front;

	float hangle; // angles to keep track of camera direction
	float vangle;
	
	camera() : pos(glm::vec3(0.0f)), hangle(-90.0f), vangle(0.0f), skip(0), prevX(0.0), prevY(0.0) { }
	camera(glm::vec3 inPos);
	void update(GLFWwindow*);
private:
	int skip;
	double prevX, prevY;
};
