#define STB_IMAGE_IMPLEMENTATION
#include "main.h"
using std::cout;
using std::cerr;
using std::endl;

using namespace glm;

enum {
	vshdr,
	fshdr,
	numShaders
};

GLuint stages[numShaders];

enum {
	vPosition = 0,
};

camera c = camera(vec3(0.0f, 0.0f, -3.0f));

struct obj {
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	unsigned int nelems;
};

obj box;

void init(shaderProgram& s) {

	glBindProgramPipeline(s.pipeline);

	stages[vshdr] = s.makeStage(GL_VERTEX_SHADER, "shader/vertex.vert");
	stages[fshdr] = s.makeStage(GL_FRAGMENT_SHADER, "shader/fragment.frag");
	
	glUseProgramStages(s.pipeline, GL_FRAGMENT_SHADER_BIT, stages[fshdr]);
	glUseProgramStages(s.pipeline, GL_VERTEX_SHADER_BIT, stages[vshdr]);

	vload::vloader objloader("models/tmapCube.dae");
	box.nelems = objloader.meshList[0].elemList.size();

	glCreateVertexArrays(1, &(box.vao));
	
	glCreateBuffers(1, &(box.vbo));
	glNamedBufferStorage(box.vbo, objloader.meshList[0].pList.size() * sizeof(vload::pt), objloader.meshList[0].pList.data(), 0);
	
	glVertexArrayVertexBuffer(box.vao, vPosition, box.vbo, 0, sizeof(vload::pt));
	glEnableVertexArrayAttrib(box.vao, vPosition);
	glVertexArrayAttribBinding(box.vao, vPosition, 0);
	glVertexArrayAttribFormat(box.vao, vPosition, 3, GL_FLOAT, GL_FALSE, 0);

	glCreateBuffers(1, &box.ebo);
	glNamedBufferStorage(box.ebo, objloader.meshList[0].elemList.size() * sizeof(unsigned int), objloader.meshList[0].elemList.data(), 0);
	glVertexArrayElementBuffer(box.vao, box.ebo);
	
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
}

void render(window& w, shaderProgram& s) {
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	c.update(w.wptr);

	mat4 modelMatrix = mat4(1.0f);
	mat4 viewMatrix = lookAt(c.pos, c.pos + c.front, vec3(0.0f, 1.0f, 0.0f));
	mat4 projectionMatrix = perspective(radians(25.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
	
	glProgramUniformMatrix4fv(stages[vshdr], 0, 1, GL_FALSE, value_ptr(modelMatrix));
	glProgramUniformMatrix4fv(stages[vshdr], 1, 1, GL_FALSE, value_ptr(viewMatrix));
	glProgramUniformMatrix4fv(stages[vshdr], 2, 1, GL_FALSE, value_ptr(projectionMatrix));
	
	glBindVertexArray(box.vao);
	glDrawElements(GL_TRIANGLES, box.nelems, GL_UNSIGNED_INT, nullptr);
}

void run(window& w) {

	shaderProgram s;

	init(s);
	while (!glfwWindowShouldClose(w.wptr)) {
		if (glfwGetKey(w.wptr, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(w.wptr, true);
		}
		
		render(w, s);

		glfwSwapBuffers(w.wptr);

		glfwPollEvents();
	}
}

int main(int argc, char **argv) {
	window w;
	run(w);
}
