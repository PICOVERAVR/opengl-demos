#pragma once
#include <iostream>
#include <string>
#include <fstream>

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

struct shader_t {
	GLenum shader_type;
	const char* path;
};

std::string readFile(std::string path);
bool compileShaders(shader_t* shaderList, int size);
