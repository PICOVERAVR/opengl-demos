#include "main.h"
using std::cout;
using std::endl; 

enum {
	vshdr,
	fshdr,
	shaderListSize
};

enum {
	vPosition = 0,
	vNormal = 1,
	vTangent = 2,
	vTexCoord = 3,
};

GLFWwindow* w;

camera c;
map terrain;

extern unit models[modelsize];
extern std::mutex modelmut;

GLuint* gpuinit(shaderProgram& s) {

	c = camera(w, glm::vec3(0.0f, 0.0f, 3.0f));

	glBindProgramPipeline(s.pipeline);

	GLuint* stages = new GLuint[shaderListSize];
	stages[vshdr] = s.makeStage({ GL_VERTEX_SHADER, "shader/vertex.vert" });
	stages[fshdr] = s.makeStage({ GL_FRAGMENT_SHADER, "shader/fragment.frag" });
	
	glUseProgramStages(s.pipeline, GL_FRAGMENT_SHADER_BIT, stages[fshdr]);
	glUseProgramStages(s.pipeline, GL_VERTEX_SHADER_BIT, stages[vshdr]);
	
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

	return stages;
}

void render(GLuint* stages, shaderProgram& s) {
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	c.update(w);

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 viewMatrix = glm::lookAt(c.pos, c.pos + c.front, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(25.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
	
	glProgramUniformMatrix4fv(stages[vshdr], 0, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glProgramUniformMatrix4fv(stages[vshdr], 1, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glProgramUniformMatrix4fv(stages[vshdr], 2, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	
	terrain.render();
	
	modelmut.lock();
	for (int i = 0; i < modelsize; i++) {
		glProgramUniformMatrix4fv(stages[vshdr], 0, 1, GL_FALSE, glm::value_ptr(models[i].modelmat));
		models[i].render();
	}
	modelmut.unlock();
	glProgramUniformMatrix4fv(stages[vshdr], 0, 1, GL_FALSE, glm::value_ptr(modelMatrix));
}

void run() {

	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	cout << "Hardware: " << glGetString(GL_RENDERER) << endl;
	cout << "Manufacturer: " << glGetString(GL_VENDOR) << endl << endl;
	
	shaderProgram s;

	std::vector<float> hm = { 
							  0.0, 0.0, 0.0, 0.0, 0.0,
							  0.0, 0.5, 0.5, 0.0, 0.0,
							  0.0, 0.5, 0.5, 0.7, 0.0,
							  0.0, 0.0, 0.7, 1.0, 0.0,
							  0.0, 0.0, 0.0, 0.0, 0.0,
							};
	
	const glm::vec4 grass = glm::vec4(0.416, 0.502, 0.0, 1.0);
	const glm::vec4 earth = glm::vec4(0.302, 0.251, 0.0, 1.0);
	const glm::vec4 snow = glm::vec4(1.0, 1.0, 1.0, 1.0);

	std::vector<glm::vec4> colorm = { earth, earth, earth, earth, earth,
									  earth, grass, grass, earth, earth,
									  earth, grass, grass, grass, earth,
									  earth, earth, grass, snow, earth,
									  earth, earth, earth, earth, earth,
									};

	GLuint* stages = gpuinit(s);
	
	terrain.load(hm, colorm, 1.0);
	models[u0].load("models/tmapCube.dae");
	std::vector<vertexinfo> const& vref = terrain.gputerrain;
	
	std::thread getinput(listen);
	getinput.detach(); // don't care if this thread gets forcefully deleted at program exit because it probably isn't in the middle of a model update

	while (!glfwWindowShouldClose(w)) {
		if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(w, true);
		}

		modelmut.lock();
		models[u0].updatepos(vref);
		modelmut.unlock();

		render(stages, s);

		glfwSwapBuffers(w);

		glfwPollEvents();
	}

	delete stages;
	terrain.destroy();
}

int main(int argc, char **argv) {
	w = systemInit();
	if (w == nullptr) {
		exit(-1); // window creation failure, error already logged.
	}
	
	run(); // a function to make sure all C++ objects are destroyed before systemDestroy() is called
	
	systemDestroy(w);
}
