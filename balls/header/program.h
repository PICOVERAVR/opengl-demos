#pragma once

#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class shaderProgram {
public:
	shaderProgram();
	~shaderProgram(); // be careful about just returning the program object since this will be called when class goes out of scope

	GLuint makeStage(GLenum stage, const char* path); // compile a shader stage and link it into a program
	
	GLuint pipeline;

private:
	std::string getShaderString(std::string path);
};
