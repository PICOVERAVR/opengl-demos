#pragma once

#include <iostream>

#include "glad.h"
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"

constexpr int screenWidth = 3840;
constexpr int screenHeight = 2160;

void interpretFramebufferStatus(GLenum error);

// create an OpenGL context with error reporting that depends on whether NDEBUG is defined or not
class window {
public:
	window();
	~window();
	
	window(const window& other);
	window& operator=(const window& other);

	window(window&& other);
	window& operator=(window&& other);
	
	GLFWwindow* wptr;
};
