/*

	NOTE: the sole purpose of this branch is for zen-like coding - write a program but never start building on it.
		if doing that, use the redbook branch.

*/

#include "main.h"
using namespace std;

enum {
	vshdr,
	fshdr,
	shaderListSize
};

enum {
	vPosition = 0,
};

GLFWwindow* w;

camera c;

GLuint vao;
GLuint vbo;

GLuint* init(shaderProgram& s) {

	c = camera(w, glm::vec3(0.0f, 0.0f, 3.0f));

	glBindProgramPipeline(s.pipeline);

	GLuint* stages = new GLuint[shaderListSize];

	stages[vshdr] = s.makeStage({ GL_VERTEX_SHADER, "shaders/vertex.vert" });
	stages[fshdr] = s.makeStage({ GL_FRAGMENT_SHADER, "shaders/fragment.frag" });
	
	glUseProgramStages(s.pipeline, GL_FRAGMENT_SHADER_BIT, stages[fshdr]);
	glUseProgramStages(s.pipeline, GL_VERTEX_SHADER_BIT, stages[vshdr]);

	GLfloat vcoords[] = {
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.0f,  0.5f, 0.0f,
	};

	glCreateVertexArrays(1, &vao);
	
	glCreateBuffers(1, &vbo);
	glNamedBufferStorage(vbo, sizeof(vcoords), vcoords, 0);

	glVertexArrayVertexBuffer(vao, vPosition, vbo, 0, sizeof(GLfloat)*3);

	glEnableVertexArrayAttrib(vao, vPosition);

	glVertexArrayAttribBinding(vao, vPosition, 0);
	glVertexArrayAttribFormat(vao, vPosition, 3, GL_FLOAT, GL_FALSE, 0);

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

	return stages;
}

void render(GLuint* stages, shaderProgram& s) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	c.update(w);

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	GLuint modelUniformLocation = glGetUniformLocation(stages[vshdr], "model");
	if (modelUniformLocation == GL_INVALID_INDEX) {
		cout << "WARNING: model uniform location not found!" << endl;
	}

	glm::mat4 viewMatrix = glm::lookAt(c.pos, c.pos + c.front, glm::vec3(0.0f, 1.0f, 0.0f));
	GLuint viewUniformLocation = glGetUniformLocation(stages[vshdr], "view");
	if (viewUniformLocation == GL_INVALID_INDEX) {
		cout << "WARNING: view uniform location not found!" << endl;
	}

	glm::mat4 projectionMatrix = glm::perspective(glm::radians(25.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
	GLuint projectionUniformLocation = glGetUniformLocation(stages[vshdr], "projection");
	if (projectionUniformLocation == GL_INVALID_INDEX) {
		cout << "WARNING: projection uniform location not found!" << endl;
	}

	glProgramUniformMatrix4fv(stages[vshdr], modelUniformLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glProgramUniformMatrix4fv(stages[vshdr], viewUniformLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glProgramUniformMatrix4fv(stages[vshdr], projectionUniformLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

int main(int argc, char **argv) {

	//std::string dummy;
	//cout << "Process ID is " << GetCurrentProcessId() << endl;
	//cout << "profiler ready?" << endl;
	//cin >> dummy;
	//if (dummy != "y" && dummy != "Y") {
	//	cout << "okay, then." << endl;
	//	return 0;
	//}

	w = systemInit();
	if (w == nullptr) {
		exit(-1); // window creation failure, error already logged.
	}

	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	cout << "Hardware: " << glGetString(GL_RENDERER) << endl;
	cout << "Manufacturer: " << glGetString(GL_VENDOR) << endl << endl;
	
	shaderProgram s;

	GLuint* stages = init(s); // if init fails, an exception will be thrown.

	while (!glfwWindowShouldClose(w)) {
		if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(w, true);
		}
		
		render(stages, s);

		glfwSwapBuffers(w);

		glfwPollEvents();
	}
	
	delete stages;

	systemDestroy(w);
}
