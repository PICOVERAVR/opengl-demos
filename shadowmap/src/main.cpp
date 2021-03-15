#include "main.h"
using std::cout;
using std::cerr;
using std::endl;

using namespace glm;

enum {
	tpotvs,
	tpotfs,
	floorvs,
	floorfs,
	shadowvs,
	shadowfs,
	lightvs,
	lightfs,
	scalecs,
	numShaders
};

GLuint stages[numShaders];

camera c;

constexpr size_t numMaps = 3;
GLuint sfb[numMaps];
GLuint sfbColorTex[numMaps];

// x range, y range, z min, z max
constexpr vec4 minMaxFrustrums[numMaps] = {vec4(1.0f, 1.0f, 4.0f, 6.0f), vec4(2.0f, 2.0f, 4.0f, 8.0f), vec4(3.0f, 3.0f, 4.0f, 30.0f)};
vec4 cascadeDepths;

mat4 shadowMatrices[numMaps];

constexpr int depthWidth = 4096;
constexpr int depthHeight = 4096;

enum { // friendly names for objects
	tpot = 0,
	tfloor = 1,
	light = 2,
	numStartObjects
};

constexpr int bpd = 16; // blocks per dimension - has to be a power of two!

// re-scales a color depth texture so that the min is 0.0 and the max is 1.0
// assumptions: depth color texture is square and a power of two, and is a 2D array of vec4's all containing the same value.
void scaleImageMinMax(GLuint tex, const int depthDim, const int blockinessPerDim) {
	
	const int numInvocations = depthDim / blockinessPerDim;
	
	static unsigned int numRuns = 0;
	static float totalTime = 0;

	glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
	
	GLsync s = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	auto start = std::chrono::steady_clock::now();
	glDispatchCompute(numInvocations, numInvocations, 1);
	glClientWaitSync(s, GL_SYNC_FLUSH_COMMANDS_BIT, 100000000); // wait up to 100ms
	auto end = std::chrono::steady_clock::now();
	
	totalTime += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	numRuns++;
	
	// print out how long the compute pass took, for fun
	if (numRuns >= 100) {
		cout << "average for " << numRuns << " runs is " << totalTime / numRuns << " ms.\n";
		numRuns = 0;
		totalTime = 0;
	}
	
	glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
}

void init(shaderProgram& s, const std::vector<std::unique_ptr<obj>>& objlist) {
	int bpx, bpy, bpz, max;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &bpx);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &bpy);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &bpz);
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max);
	
	cout << "max dims: " << bpx << ", " << bpy << ", " << bpz << ", total max: " << max << endl;
	
	GLuint ssboBuf;
	glCreateBuffers(1, &ssboBuf);
	glNamedBufferStorage(ssboBuf, sizeof(vec2) * (4096 / bpd) * (4096 / bpd), nullptr, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboBuf);

	cascadeDepths = vec4(0.0f);
	for (size_t i = 0; i < numMaps; i++) {
		cascadeDepths[i] = -minMaxFrustrums[i].w;
	}
	
	c = camera(vec3(0.0f, 0.0f, -3.0f));

	glBindProgramPipeline(s.pipeline);

	stages[tpotvs] = s.makeStage(GL_VERTEX_SHADER, "shader/tpot.vert");
	stages[tpotfs] = s.makeStage(GL_FRAGMENT_SHADER, "shader/tpot.frag");
	
	stages[floorvs] = s.makeStage(GL_VERTEX_SHADER, "shader/floor.vert");
	stages[floorfs] = s.makeStage(GL_FRAGMENT_SHADER, "shader/floor.frag");

	stages[shadowvs] = s.makeStage(GL_VERTEX_SHADER, "shader/shadow.vert");
	stages[shadowfs] = s.makeStage(GL_FRAGMENT_SHADER, "shader/shadow.frag");
	
	stages[lightvs] = s.makeStage(GL_VERTEX_SHADER, "shader/light.vert");
	stages[lightfs] = s.makeStage(GL_FRAGMENT_SHADER, "shader/light.frag");

	stages[scalecs] = s.makeStage(GL_COMPUTE_SHADER, "shader/scale_image.comp");
	glUseProgramStages(s.pipeline, GL_COMPUTE_SHADER_BIT, stages[scalecs]);

	vload::vloader objloader("models/utah-teapot.dae");
	objlist[tpot]->load(objloader);

	vload::vloader lightloader("models/tmapSphere.dae");
	objlist[light]->load(lightloader);
	
	vload::vloader the_floorloader("models/tmapFlat.dae");
	objlist[tfloor]->load(the_floorloader);
	
	GLuint samp;
	glCreateSamplers(1, &samp);
	
	glSamplerParameteri(samp, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(samp, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	constexpr vec4 depthClampVal = vec4(1.0f);
	glSamplerParameterfv(samp, GL_TEXTURE_BORDER_COLOR, value_ptr(depthClampVal));
	
	glSamplerParameteri(samp, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glSamplerParameteri(samp, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glSamplerParameteri(samp, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glSamplerParameteri(samp, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	
	for (size_t i = 0; i < numMaps; i++) {
		GLuint sfbTex;
		glCreateTextures(GL_TEXTURE_2D, 1, &sfbTex);
		glTextureStorage2D(sfbTex, 1, GL_DEPTH_COMPONENT32F, depthWidth >> i, depthHeight >> i);
		
		glCreateTextures(GL_TEXTURE_2D, 1, &(sfbColorTex[i]));
		glTextureStorage2D(sfbColorTex[i], 1, GL_RGBA8, depthWidth >> i, depthHeight >> i);

		glCreateFramebuffers(1, &(sfb[i]));
		glNamedFramebufferTexture(sfb[i], GL_DEPTH_ATTACHMENT, sfbTex, 0);
		glNamedFramebufferTexture(sfb[i], GL_COLOR_ATTACHMENT0, sfbColorTex[i], 0);

		interpretFramebufferStatus(glCheckNamedFramebufferStatus(sfb[i], GL_DRAW_FRAMEBUFFER));
		interpretFramebufferStatus(glCheckNamedFramebufferStatus(sfb[i], GL_READ_FRAMEBUFFER));
		
		glBindTextureUnit(i, sfbTex); // for reading the texture afterwards
		glBindSampler(i, samp);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_FRAMEBUFFER_SRGB);

	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
}

void render(shaderProgram& s, window& w, const std::vector<std::unique_ptr<obj>>& objlist) {
	
	c.update(w.wptr);
	
	static vec3 light_pos;
	static float langle = 0;
	static float dlangle = 0.01f;

	langle += dlangle;
	if (langle > 2 * 3.14159) {
		langle = langle - 2 * 3.14159;
	}
	light_pos = vec3(6.0f * cosf(langle), 3.0f, 6.0f * sinf(langle));
	
	if (glfwGetKey(w.wptr, GLFW_KEY_O) == GLFW_PRESS) {
		dlangle = 0.01f;
	}

	if (glfwGetKey(w.wptr, GLFW_KEY_P) == GLFW_PRESS) {
		dlangle = 0.0f;
	}
	
	mat4 modelMatrix = scale(mat4(1.0f), vec3(0.075f, 0.075f, 0.075f));
	mat4 floorModelMatrix = scale(mat4(1.0f), vec3(8.0f));
	mat4 lightvsiewMatrix = lookAt(light_pos, vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);
	GLfloat depth_clear = 1.0f;
	GLfloat color_clear[4] = {1.0f, 1.0f, 1.0f, 1.0f};

	for (size_t i = 0; i < numMaps; i++) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, sfb[i]);
		glClearNamedFramebufferfv(sfb[i], GL_DEPTH, 0, &(depth_clear));
		glClearNamedFramebufferfv(sfb[i], GL_COLOR, 0, color_clear);
		glViewport(0, 0, depthWidth >> i, depthHeight >> i);

		glUseProgramStages(s.pipeline, GL_FRAGMENT_SHADER_BIT, stages[shadowfs]);
		glUseProgramStages(s.pipeline, GL_VERTEX_SHADER_BIT, stages[shadowvs]);
		
		shadowMatrices[i] = frustum(-minMaxFrustrums[i].x, minMaxFrustrums[i].x, -minMaxFrustrums[i].y, minMaxFrustrums[i].y, minMaxFrustrums[i].z, minMaxFrustrums[i].w);

		glProgramUniformMatrix4fv(stages[shadowvs], 0, 1, GL_FALSE, value_ptr(modelMatrix));
		glProgramUniformMatrix4fv(stages[shadowvs], 1, 1, GL_FALSE, value_ptr(lightvsiewMatrix));
		glProgramUniformMatrix4fv(stages[shadowvs], 2, 1, GL_FALSE, value_ptr(shadowMatrices[i]));

		glBindVertexArray(objlist[tpot]->vao);
		glDisableVertexArrayAttrib(objlist[tpot]->vao, vNormal);
		objlist[tpot]->render();
		glEnableVertexArrayAttrib(objlist[tpot]->vao, vNormal);
		
		glProgramUniformMatrix4fv(stages[shadowvs], 0, 1, GL_FALSE, value_ptr(floorModelMatrix));
		
		glEnable(GL_CULL_FACE);
		objlist[tfloor]->render();
		glDisable(GL_CULL_FACE);
	}
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glNamedFramebufferDrawBuffer(0, GL_BACK_LEFT);
	glViewport(0, 0, screenWidth, screenHeight);
	glDisable(GL_POLYGON_OFFSET_FILL);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 projectionMatrix = perspective(glm::radians(25.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
	mat4 viewMatrix = lookAt(c.pos, c.pos + c.front, vec3(0.0f, 1.0f, 0.0f));

	glUseProgramStages(s.pipeline, GL_FRAGMENT_SHADER_BIT, stages[tpotfs]);
	glUseProgramStages(s.pipeline, GL_VERTEX_SHADER_BIT, stages[tpotvs]);

	glProgramUniformMatrix4fv(stages[tpotvs], 0, 1, GL_FALSE, value_ptr(modelMatrix));
	glProgramUniformMatrix4fv(stages[tpotvs], 1, 1, GL_FALSE, value_ptr(viewMatrix));
	glProgramUniformMatrix4fv(stages[tpotvs], 2, 1, GL_FALSE, value_ptr(projectionMatrix));
	
	vec3 light_pos_eye = vec3(vec4(light_pos, 1.0f));
	vec3 eye_pos_eye = vec3(vec4(c.pos, 1.0f));
	constexpr vec3 lightColor = vec3(1.0f, 1.0f, 0.6f);
	
	glProgramUniform3fv(stages[tpotfs], 0, 1, value_ptr(light_pos_eye));
	glProgramUniform3fv(stages[tpotfs], 1, 1, value_ptr(eye_pos_eye));
	glProgramUniform3fv(stages[tpotfs], 2, 1, value_ptr(lightColor));

	objlist[tpot]->render();
	
	glProgramUniformMatrix4fv(stages[floorvs], 0, 1, GL_FALSE, value_ptr(floorModelMatrix));
	glProgramUniformMatrix4fv(stages[floorvs], 1, 1, GL_FALSE, value_ptr(viewMatrix));
	glProgramUniformMatrix4fv(stages[floorvs], 2, 1, GL_FALSE, value_ptr(projectionMatrix));
	glProgramUniformMatrix4fv(stages[floorvs], 3, 1, GL_FALSE, value_ptr(lightvsiewMatrix));
	
	mat4 psm_matrix = glm::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, cascadeDepths[numMaps - 1]);
	glProgramUniformMatrix4fv(stages[floorvs], 4, 1, GL_FALSE, value_ptr(psm_matrix));
	
	glProgramUniform3fv(stages[floorfs], 0, 1, value_ptr(lightColor));
	glProgramUniform4fv(stages[floorfs], 1, 1, value_ptr(cascadeDepths));
	glProgramUniformMatrix4fv(stages[floorfs], 2, numMaps, GL_FALSE, (GLfloat*)shadowMatrices);
	
	glUseProgramStages(s.pipeline, GL_FRAGMENT_SHADER_BIT, stages[floorfs]);
	glUseProgramStages(s.pipeline, GL_VERTEX_SHADER_BIT, stages[floorvs]);
	
	glEnable(GL_CULL_FACE);
	objlist[tfloor]->render();

	glUseProgramStages(s.pipeline, GL_FRAGMENT_SHADER_BIT, stages[lightfs]);
	glUseProgramStages(s.pipeline, GL_VERTEX_SHADER_BIT, stages[lightvs]);
	
	glProgramUniformMatrix4fv(stages[lightvs], 0, 1, GL_FALSE, value_ptr(scale(translate(mat4(1.0f), light_pos), vec3(0.3f))));
	glProgramUniformMatrix4fv(stages[lightvs], 1, 1, GL_FALSE, value_ptr(viewMatrix));
	glProgramUniformMatrix4fv(stages[lightvs], 2, 1, GL_FALSE, value_ptr(projectionMatrix));
	glProgramUniform3fv(stages[lightfs], 0, 1, value_ptr(lightColor));
	objlist[light]->render();
	
	glDisable(GL_CULL_FACE);

	scaleImageMinMax(sfbColorTex[0], 4096, bpd);
	scaleImageMinMax(sfbColorTex[1], 2048, bpd);
	scaleImageMinMax(sfbColorTex[2], 1024, bpd);
	
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT);
	
	// draw the depth buffers onto the bottom left of the screen
	// glBlitNamedFramebuffer can't draw from the depth buffer to the color buffer
	// so we draw the depth data to the texture and to color_attachment0, so we can just blit at the end!
	for (size_t i = 0; i < numMaps; i++) {
		glBlitNamedFramebuffer(sfb[i], 0, 0, 0, depthWidth >> i, depthHeight >> i, 640 * i + 50, 50, 640 * (i + 1) + 50, 480 + 50, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
}

// OpenGL context exists when this is run
void run(window& w) {

	shaderProgram s;
	
	std::vector<std::unique_ptr<obj>> objlist;
	
	for (unsigned int i = 0; i < numStartObjects; i++) {
		objlist.push_back(std::make_unique<obj>());
	}

	init(s, objlist);

	while (!glfwWindowShouldClose(w.wptr)) {
		glfwPollEvents();
		
		if (glfwGetKey(w.wptr, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(w.wptr, true);
		}
		
		render(s, w, objlist);

		glfwSwapBuffers(w.wptr);
	}
}

int main() {
	window w; // setup is complex, likely needs to be inside main
	run(w);
}
