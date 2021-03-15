#include "main.h"

#include <memory>

using namespace std;
using namespace load;

enum {
	svshdr,
	sfshdr,
	shaderListSize,
};

enum {
	skybox,
	object,
	numObjects
};

enum {
	NO_COLOR, // do / do not use color when drawing
	COLOR
};

GLFWwindow* w;

camera c;
int rendersize = 0; // number of cubes to render
const int reload = 25; // interval at which to load new geometry

const int blocksize = 32; // number of blocks in flight at any given time
GLuint blocklist[blocksize]; // circular buffer of blocks to keep track of
int active = 0; // the active block of cubes we are rendering
const int lookahead = 16; // how many blocks to render in advance

bool rendered = false; // has the scenery already been rendered?

const int scale = 4;

extern int screenWidth;
extern int screenHeight;

// loads cubes at the center of pos
GLuint loadCubes(int xlen, int zlen, glm::vec3 pos) {
	GLuint arrBuf; // buffer to hold array of 4x4 matrices that adjust position vector up/down, as well as horizontal location
	glCreateBuffers(1, &arrBuf);
	glBindBuffer(GL_ARRAY_BUFFER, arrBuf);
	
	glm::mat4* mptr = new glm::mat4[(int64_t)xlen * (int64_t)zlen];
	
	int index = 0;
	
	int xwidth = xlen / 2;
	int zwidth = zlen / 2;

	for (float i = -xwidth / scale; i <= xwidth / scale; i += (1.0f / scale)) { // assuming box has length 1
		for (float j = -zwidth / scale; j <= zwidth / scale; j += (1.0f / scale)) {
	
			glm::mat4 temp = glm::translate(glm::mat4(1.0f), pos);

			float randi = ((rand() % 256) - 128.0) / 128.0;
			float pos = 5 * sinf((j / (float)zwidth) * 2*3.14159265358 + 3*3.14159265358/2) + randi + 4;

			temp = glm::translate(temp, glm::vec3(i, pos, j)); // random height, predictable grid
	
			temp = glm::scale(temp, glm::vec3(1.0f / scale, 1.0f / scale, 1.0f / scale));
			mptr[index++] = temp;
		}
	}

	rendersize = index;

    glNamedBufferStorage(arrBuf, sizeof(glm::mat4) * rendersize, mptr, 0);
	delete[] mptr;
	
	for (int i = 0; i < 4; i++) {
		int nloc = 4 + i; // model matrix location
		glEnableVertexAttribArray(nloc);
		glVertexAttribPointer(nloc, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(sizeof(glm::vec4) * i));
		glVertexAttribDivisor(nloc, 1);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return arrBuf;
}

void enableCubes(GLuint buf) {
	// "A call to glBindBuffer to set the GL_ARRAY_BUFFER binding is NOT modifying the current VAO's state!"
	// https://www.khronos.org/opengl/wiki/Vertex_Specification#Vertex_Buffer_Object
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	for (int i = 0; i < 4; i++) {
		glVertexAttribPointer(i + 4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(sizeof(glm::vec4) * i));
	}
}

void freeCubes(GLuint* buf) {
	//cout << "freeing buffer " << buf << endl;
	glInvalidateBufferData(*buf);
	glDeleteBuffers(1, buf);
}

GLuint* init(vector<model*>& modelList, shaderProgram& s) {

	srand((unsigned int)glfwGetTime());

	c = camera(w, glm::vec3(0.0f, 0.0f, 3.0f));

	glBindProgramPipeline(s.pipeline);

	GLuint* stages = new GLuint[shaderListSize];

	// no need to use different stages when loading stuff since no uniforms are written.
	vloader skybox("models/tmapCube.dae");
	model *temp = new model;
	temp->load(skybox.meshList);
	modelList.push_back(temp);

	stages[svshdr] = s.makeStage({ GL_VERTEX_SHADER, "shaders/special.vert" });
	stages[sfshdr] = s.makeStage({ GL_FRAGMENT_SHADER, "shaders/special.frag" });

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // almost black
	
	c.pos.x += 0.01;
	c.pos.y += 2;
	c.hangle = 0.0f;
	
	for (int i = active; i < lookahead; i++) {
		blocklist[active + i] = loadCubes(100, 50, glm::vec3(c.pos.x + 10*i, 0, 2.5));
	}

	return stages;
}

void render(GLuint* stages, shaderProgram& s, vector<model*>& modelList) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glm::mat4 modelMatrix(1.0f), viewMatrix(1.0f), projectionMatrix(1.0f);

	c.update(w);
	c.pos.x += 0.5f;
	viewMatrix = glm::lookAt(c.pos, c.pos + c.front, glm::vec3(0.0f, 1.0f, 0.0f));

	projectionMatrix = glm::perspective(glm::radians(25.0f), (float)screenWidth / (float)screenHeight, 0.1f, 150.0f);

	glVertexAttrib4f(vColor, 1.0f, 1.0f, 1.0f, 1.0f);
	
	glm::mat4 cubeModel = modelMatrix;

	glUseProgramStages(s.pipeline, GL_VERTEX_SHADER_BIT, stages[svshdr]);
	glUseProgramStages(s.pipeline, GL_FRAGMENT_SHADER_BIT, stages[sfshdr]);

	GLuint viewUniformLocation = glGetUniformLocation(stages[svshdr], "view");
	if (viewUniformLocation == GL_INVALID_INDEX) {
		cout << "WARNING: view uniform location not found!" << endl;
	}

	GLuint projectionUniformLocation = glGetUniformLocation(stages[svshdr], "projection");
	if (projectionUniformLocation == GL_INVALID_INDEX) {
		cout << "WARNING: projection uniform location not found!" << endl;
	}

	glProgramUniformMatrix4fv(stages[svshdr], viewUniformLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glProgramUniformMatrix4fv(stages[svshdr], projectionUniformLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	if ((int)c.pos.x % reload == 0) {
		if (!rendered) {
			//cout << "Loading more geometry at position " << c.pos.x << endl;
			
			freeCubes(blocklist + active);
			blocklist[(active + lookahead) % blocksize] = loadCubes(100, 50, glm::vec3((c.pos.x + 10*lookahead), 0, 2.5)); // offset the camera a while ago?
			active = (++active % blocksize);

			rendered = true;
		}
	}
	else {
		rendered = false;
	}

	if (glfwGetKey(w, GLFW_KEY_TAB) != GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glVertexAttrib4f(vColor, 0.35f, 0.65f, 0.15f, 1.0f);

		for (int i = 0; i < lookahead; i++) {
			enableCubes(blocklist[(active + i) % blocksize]);
			modelList[0]->drawInstanced(NO_COLOR, rendersize);
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	glVertexAttrib4f(vColor, 0.05f, 0.05f, 0.05f, 1.0f);
	for (int i = 0; i < lookahead; i++) {
		enableCubes(blocklist[(active + i) % blocksize]);
		modelList[0]->drawInstanced(NO_COLOR, rendersize);
	}
}

int main(int argc, char **argv) {

	//std::string dummy;
	//cout << "RenderDoc ready?" << endl;
	//cin >> dummy;
	//if (dummy != "y" && dummy != "Y") {
	//	cout << "okay, then." << endl;
	//	return 0;
	//}

	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // see main.h to turn on, output is in debug panel.
	// NOTE: leak checker will catch modelList as not being freed, but modelList is freed when it goes out of scope.
	// force this to happen by calling modelList.~vector() before DumpMemoryLeaks().

	w = systemInit();
	if (w == nullptr) {
		exit(-1); // window creation failure, error already logged.
	}

	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	cout << "Hardware: " << glGetString(GL_RENDERER) << endl;
	cout << "Manufacturer: " << glGetString(GL_VENDOR) << endl << endl;

	auto modelPointer = make_unique<vector<model*>>();
	shaderProgram s;

	GLuint* stages = init(*modelPointer, s);
	if (stages == nullptr) { // if a valid program pipeline was not created, do nothing
		return -1;
	}

	while (!glfwWindowShouldClose(w)) {
		if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(w, true); // close window
		}
		
		render(stages, s, *modelPointer);

		glfwSwapBuffers(w);

		glfwPollEvents();
	}
	
	delete stages;
	for (auto i = modelPointer->begin(); i != modelPointer->end(); i++) {
		delete* i;
	}

	systemDestroy(w);

	//_CrtDumpMemoryLeaks();
}

/*
			This should be done in a very professional C++ manner since it's good practice

		CRITICAL TODO:
			- cannot have OpenGL destructors running after the GLFW window has closed. All OpenGL objects need to be destroyed before calling systemDestroy().
			- loadCubes breaks for dimensions that don't divide by two evenly - drops value, but OpenGL writes to that value

		C++-ify:
			- convert old for-loops into iterator ones when using vectors
			- use unique pointers 
			- use const at the end of method declarations
			- find C stuff and strip it out
			- might want to throw exceptions on serious stuff, like a model failing to load.  Program should crash if this fails, and it currently does.
			- namespace everything, use longer names to avoid collisions with things

		Misc:
			- movement speed dependant on framerate
*/
