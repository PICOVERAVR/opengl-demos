#include "program.h"
using namespace std;

string shaderProgram::getShaderString(string path) {
	string content;
	ifstream srcStream(path, ios::in);
	if (!srcStream.is_open()) {
		cerr << "Could not open file " << path << "!" << endl;
		return "";
	}
	string line = "";
	while (!srcStream.eof()) {
		getline(srcStream, line);
		content.append(line + "\n");
	}
	srcStream.close();
	return content;
}

shaderProgram::shaderProgram() {
	glCreateProgramPipelines(1, &pipeline);
}

shaderProgram::~shaderProgram() {
	glDeleteProgramPipelines(1, &pipeline);
}

// compile and link a shader stage with one call, returns the index into the internal array with which to keep track of the stage
GLuint shaderProgram::makeStage(GLenum stage, const char* path) {
	
	string shaderSrc = getShaderString(path);
	if (!shaderSrc.compare("")) {
		return 0;
	}
	const GLchar* cstr = shaderSrc.c_str();

	cout << "Compilation output of " << path << ": " << endl;
	GLuint program = glCreateShaderProgramv(stage, 1, &cstr);

	GLint len;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

	if (len > 0) { // don't know if the driver is reporting errors or hints, display it all (asusming that if compilation fails, an error will be reported)
		GLchar* log = new GLchar[(GLint64)len + 1];
		glGetProgramInfoLog(program, len, NULL, log);

		cout << log << endl;
		delete[] log;

		GLint compiled;
		GLint linked;

		glGetShaderiv(program, GL_COMPILE_STATUS, &compiled);
		glGetShaderiv(program, GL_LINK_STATUS, &linked);

		if (!compiled || !linked) {
			cerr << "Shader construction failed!" << endl;
			return 0;
		}
	}
	else {
		cout << "Nothing to report." << endl;
	}

	return program;
}
