#pragma once

#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// list of compiled shaders is external because I didn't want to totally encapsulate this class.

struct shaderInfo {
	GLenum stage; // shader stage to compile
	const char* path; // path to shader file
};

class shaderProgram {
public:
	shaderProgram();
	~shaderProgram(); // be careful about just returning the program object since this will be called when class goes out of scope

	GLuint makeStage(shaderInfo s); // compile a shader stage and link it into a program
	
	GLuint pipeline;

private:
	std::string getShaderString(std::string path);
};