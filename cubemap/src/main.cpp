#include "main.h"
using namespace std;
using namespace load;

enum {
	vshdr,
	vcubeshdr,
	fshdr,
	ftexshdr,
	fcubeshdr,
	pipeline,
	shaderListSize,
};

enum {
	object,
	skybox,
};

enum {
	NO_COLOR, // do / do not use color when drawing
	COLOR
};

GLFWwindow* w; // ok for this to be global since it's used by a decent amount of stuff and there is only one of it ever

camera c; // only one camera for now

GLuint* init(vector<model>& modelList, shaderProgram& s) {

	c = camera(w, glm::vec3(0.0f, 0.0f, 3.0f));

	glBindProgramPipeline(s.pipeline);

	GLuint* stages = new GLuint[shaderListSize];

	stages[vshdr] = s.makeStage({ GL_VERTEX_SHADER, "shaders/vertex.vert" });
	stages[vcubeshdr] = s.makeStage({ GL_VERTEX_SHADER, "shaders/vertCube.vert" });
	stages[fshdr] = s.makeStage({ GL_FRAGMENT_SHADER, "shaders/fragment.frag" });
	stages[ftexshdr] = s.makeStage({ GL_FRAGMENT_SHADER, "shaders/fragTex.frag" });
	stages[fcubeshdr] = s.makeStage({ GL_FRAGMENT_SHADER, "shaders/fragCube.frag" });

	model m;
	model cubemap;
	model test;

	modelList.push_back(m);
	modelList.push_back(cubemap);
	modelList.push_back(test);

	// no need to use different stages when loading stuff since no uniforms are accessed.

	string pathList[6] = {
	"images/right.jpg",
	"images/left.jpg",
	"images/top.jpg",
	"images/bottom.jpg",
	"images/front.jpg",
	"images/back.jpg",
	};

	if (!modelList[skybox].loadModel("models/tmapCube.dae")) {
		cerr << "Cubemap loading error!" << endl;
		return nullptr;
	}

	GLuint cbinding = glGetUniformLocation(stages[fcubeshdr], "skybox");
	if (!modelList[skybox].loadTextureArray(GL_TEXTURE_CUBE_MAP, pathList, 6, cbinding)) {
		cerr << "Skybox texture loading error!" << endl;
		return nullptr;
	}

	if (!modelList[object].loadModel("models/ortho2.dae")) {
		cerr << "Model loading error!" << endl;
		return nullptr;
	}

	//string texPath = "C:\\Users\\kyle\\Desktop\\2Dart\\bump_map.jpg";
	//GLint binding = glGetUniformLocation(stages[ftexshdr], "tex"); // figure out which texture unit the texture will be bound to, note that if a texture is optimized out things will break

	//if (!modelList[object].loadTexture(GL_TEXTURE_2D, texPath, binding)) {
	//	cerr << "Texture loading error!" << endl;
	//	return nullptr;
	//}

	GLuint cbindingo = glGetUniformLocation(stages[ftexshdr], "tex2");
	if (!modelList[object].loadTextureArray(GL_TEXTURE_CUBE_MAP, pathList, 6, cbindingo)) {
		cerr << "Object cubemap texture loading error!" << endl;
		return nullptr;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glClearColor(0.05f, 0.05f, 0.05f, 1.0f); // almost black

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	GLint msNum;
	glGetIntegerv(GL_SAMPLES, &msNum);
	cout << "Multisampled samples per pixel: " << msNum << endl;

	return stages;
}

void render(GLuint* stages, shaderProgram& s, vector<model>& modelList) {
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	GLuint modelCubeUniformLocation = glGetUniformLocation(stages[vcubeshdr], "model");
	if (modelCubeUniformLocation == GL_INVALID_INDEX) {
		cout << "WARNING: model uniform location not found!" << endl;
	}

	GLuint viewCubeUniformLocation = glGetUniformLocation(stages[vcubeshdr], "view");
	if (viewCubeUniformLocation == GL_INVALID_INDEX) {
		cout << "WARNING: view uniform location not found!" << endl;
	}

	GLuint projectionCubeUniformLocation = glGetUniformLocation(stages[vcubeshdr], "projection");
	if (projectionCubeUniformLocation == GL_INVALID_INDEX) {
		cout << "WARNING: projection uniform location not found!" << endl;
	}

	glm::mat4 model(1.0f), view(1.0f), projection(1.0f);

	c.update(w);
	view = glm::lookAt(c.pos, c.pos + c.front, glm::vec3(0.0f, 1.0f, 0.0f));

	projection = glm::perspective(glm::radians(25.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

	glVertexAttrib4f(vColor, 1.0f, 1.0f, 1.0f, 1.0f);
	
	glm::mat4 cubeModel = model;

	glProgramUniformMatrix4fv(stages[vcubeshdr], modelCubeUniformLocation, 1, GL_FALSE, glm::value_ptr(cubeModel)); // scale skybox up
	glProgramUniformMatrix4fv(stages[vcubeshdr], viewCubeUniformLocation, 1, GL_FALSE, glm::value_ptr(view));
	glProgramUniformMatrix4fv(stages[vcubeshdr], projectionCubeUniformLocation, 1, GL_FALSE, glm::value_ptr(projection));

	glUseProgramStages(s.pipeline, GL_FRAGMENT_SHADER_BIT, stages[fcubeshdr]);
	glUseProgramStages(s.pipeline, GL_VERTEX_SHADER_BIT, stages[vcubeshdr]);
	modelList[skybox].draw(NO_COLOR);
	glUseProgramStages(s.pipeline, GL_FRAGMENT_SHADER_BIT, stages[ftexshdr]);
	glUseProgramStages(s.pipeline, GL_VERTEX_SHADER_BIT, stages[vshdr]);

	GLuint camloc = glGetUniformLocation(stages[ftexshdr], "campos");
	if (projectionCubeUniformLocation == GL_INVALID_INDEX) {
		cout << "WARNING: camera uniform location not found!" << endl;
	}

	glProgramUniform3fv(stages[ftexshdr], camloc, 1, glm::value_ptr(c.pos));

	GLuint modelUniformLocation = glGetUniformLocation(stages[vshdr], "model"); // uniforms are seperate since different program pipelines are used
	if (modelUniformLocation == GL_INVALID_INDEX) {
		cout << "WARNING: model uniform location not found!" << endl;
	}

	GLuint viewUniformLocation = glGetUniformLocation(stages[vshdr], "view");
	if (viewUniformLocation == GL_INVALID_INDEX) {
		cout << "WARNING: view uniform location not found!" << endl;
	}

	GLuint projectionUniformLocation = glGetUniformLocation(stages[vshdr], "projection");
	if (projectionUniformLocation == GL_INVALID_INDEX) {
		cout << "WARNING: projection uniform location not found!" << endl;
	}

	model = glm::rotate(model, glm::radians(20.0f) * (float)glfwGetTime() * 5, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-65.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.03f));

	glProgramUniformMatrix4fv(stages[vshdr], modelUniformLocation, 1, GL_FALSE, glm::value_ptr(model));
	glProgramUniformMatrix4fv(stages[vshdr], viewUniformLocation, 1, GL_FALSE, glm::value_ptr(view));
	glProgramUniformMatrix4fv(stages[vshdr], projectionUniformLocation, 1, GL_FALSE, glm::value_ptr(projection));

	if (glfwGetKey(w, GLFW_KEY_TAB) != GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glVertexAttrib4f(vColor, 0.35f, 0.65f, 0.15f, 1.0f);

		modelList[object].draw(NO_COLOR);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	glVertexAttrib4f(vColor, 0.35f, 0.45f, 0.15f, 1.0f);
	modelList[object].draw(COLOR);
}

int main(int argc, char **argv) {
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // see main.h, for memory leaks

	w = systemInit();
	if (w == nullptr) {
		exit(-1); // window creation failure, error already logged.
	}

	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	cout << "Hardware: " << glGetString(GL_RENDERER) << endl;
	cout << "Manufacturer: " << glGetString(GL_VENDOR) << endl << endl;

	vector<model> modelList; // this is a terrible solution
	shaderProgram s;

	GLuint* stages = init(modelList, s);
	if (stages == nullptr) { // if a valid program pipeline was not created, do nothing
		return -1;
	}

	while (!glfwWindowShouldClose(w)) {
		if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(w, true); // close window
		}

		render(stages, s, modelList);

		glfwSwapBuffers(w);
		glfwPollEvents();
	}
	
	delete stages;
	systemDestroy(w);
}

/*

	TODO:
		
			This should be done in a very professional C++ manner since it's good practice

		cache texture data so they aren't re-loaded over and over again
			- need a global string->texture data table, if a texture is going to be loaded hash it and check if it is in the cache

		start removing warnings that are created by various things]
			- glfw.pdb error unavoidable since GLFW does not ship with either source or source information (.pdb files)
		
		convert old for-loops into iterator ones when using vectors
		
		why do cubemaps have to be rendered before the object is rendered in order for stuff to work?

		fix the model class being REALLY fragile - can't copy or assign it anywhere because the underlying OpenGL objects aren't copied and that would be hella expensive? is it?
			- allocates resources but rule of 3 isn't followed, leads to problems when copying stuff around

		samplers/textures aren't destroyed when application quits
			- move handles into something that's configurable by the user

		continue learning OpenGL

		learn 4.6 stuff
*/
