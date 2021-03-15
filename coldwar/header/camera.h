#pragma once

#include "window.h" // include this in order to get access to window dimensions
#include <cmath> // cmath is weird here and doesn't have M_PI, avoid it by defining pi ourselves

class camera {
public:
	glm::vec3 pos;
	glm::vec3 front;

	float hangle; // angles to keep track of camera direction
	float vangle;
	
	camera() : pos(glm::vec3(0.0f)), front(glm::vec3(0.0f, 0.0f, -1.0f)), hangle(-90.0f), vangle(0.0f), skip(0), prevX(0.0), prevY(0.0) { }
	camera(GLFWwindow *w, glm::vec3 inPos);
	void update(GLFWwindow*);
private:
	int skip;
	double prevX, prevY;
};
