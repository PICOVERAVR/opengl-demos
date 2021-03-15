/*
	creates an OpenGL context and populates it with the proper config
*/

#pragma once

#include <iostream>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"

const int screenWidth = 3840;
const int screenHeight = 2160;
GLFWwindow* systemInit(); // create GLFW3 context and load OpenGL functions
void interpretFramebufferStatus(GLenum fbresult);
void systemDestroy(GLFWwindow*); // destroy context
