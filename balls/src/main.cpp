#define STB_IMAGE_IMPLEMENTATION
#include "main.h"
using namespace std;

enum {
	vshdr,
	fshdr,
	the_floor_vshdr,
	the_floor_fshdr,
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

struct obj {
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	unsigned int elems;
} ball;

struct tex_obj : obj {
	GLuint ftex;
	GLuint64 handle;
} the_floor;

GLuint stages[shaderListSize];

const int num_balls = 8;
const float ball_mass = 1.0f;
glm::vec3 ball_vel[num_balls];
glm::vec3 ball_pos[2][num_balls];
size_t active_pos_buf = 0;

void init(shaderProgram& s) {

	c = camera(w, glm::vec3(0.0f, 4.0f, -8.0f));

	glBindProgramPipeline(s.pipeline);

	stages[vshdr] = s.makeStage(GL_VERTEX_SHADER, "shader/vertex.vert");
	stages[fshdr] = s.makeStage(GL_FRAGMENT_SHADER, "shader/fragment.frag");

	stages[the_floor_vshdr] = s.makeStage(GL_VERTEX_SHADER, "shader/the_floor.vert");
	stages[the_floor_fshdr] = s.makeStage(GL_FRAGMENT_SHADER, "shader/the_floor.frag");
	
	glUseProgramStages(s.pipeline, GL_FRAGMENT_SHADER_BIT, stages[fshdr]);
	glUseProgramStages(s.pipeline, GL_VERTEX_SHADER_BIT, stages[vshdr]);

	vload::vloader objloader("models/tmapSphere.dae");
	ball.elems = objloader.meshList[0].elemList.size();

	glCreateVertexArrays(1, &(ball.vao));
	
	glCreateBuffers(1, &(ball.vbo));
	glNamedBufferStorage(ball.vbo, objloader.meshList[0].pList.size() * sizeof(vload::pt), objloader.meshList[0].pList.data(), 0);
	
	glVertexArrayVertexBuffer(ball.vao, vPosition, ball.vbo, 0, sizeof(vload::pt));
	glEnableVertexArrayAttrib(ball.vao, vPosition);
	glVertexArrayAttribBinding(ball.vao, vPosition, 0);
	glVertexArrayAttribFormat(ball.vao, vPosition, 3, GL_FLOAT, GL_FALSE, 0);
	
	glCreateBuffers(1, &(ball.ebo));
	glNamedBufferStorage(ball.ebo, objloader.meshList[0].elemList.size() * sizeof(unsigned int), objloader.meshList[0].elemList.data(), 0);
	glVertexArrayElementBuffer(ball.vao, ball.ebo);
	
	vload::vloader floorloader("models/tmapFlat.dae");
	the_floor.elems = floorloader.meshList[0].elemList.size();

	glCreateVertexArrays(1, &(the_floor.vao));
	
	glCreateBuffers(1, &(the_floor.vbo));
	glNamedBufferStorage(the_floor.vbo, floorloader.meshList[0].pList.size() * sizeof(vload::pt), floorloader.meshList[0].pList.data(), 0);

	glVertexArrayVertexBuffer(the_floor.vao, vPosition, the_floor.vbo, 0, sizeof(vload::pt));
	glEnableVertexArrayAttrib(the_floor.vao, vPosition);
	glVertexArrayAttribBinding(the_floor.vao, vPosition, 0);
	glVertexArrayAttribFormat(the_floor.vao, vPosition, 3, GL_FLOAT, GL_FALSE, 0);

	glVertexArrayVertexBuffer(the_floor.vao, vTexCoord, the_floor.vbo, 9 * sizeof(GLfloat), sizeof(vload::pt));
	glEnableVertexArrayAttrib(the_floor.vao, vTexCoord);
	glVertexArrayAttribBinding(the_floor.vao, vTexCoord, 3);
	glVertexArrayAttribFormat(the_floor.vao, vTexCoord, 2, GL_FLOAT, GL_FALSE, 0);

	glCreateBuffers(1, &(the_floor.ebo));
	glNamedBufferStorage(the_floor.ebo, floorloader.meshList[0].elemList.size() * sizeof(unsigned int), floorloader.meshList[0].elemList.data(), 0);
	glVertexArrayElementBuffer(the_floor.vao, the_floor.ebo);
	
	int x, y, n;
	uint8_t* data = stbi_load("textures/sand.jpg", &x, &y, &n, 0);
	if (data == nullptr) {
		cerr << "Cannot find texture!" << endl;
		abort();
	}

	GLuint temp_load;
	glCreateBuffers(1, &temp_load);
	glNamedBufferStorage(temp_load, x * y * sizeof(uint8_t) * n, data, 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, temp_load);
	glCreateTextures(GL_TEXTURE_2D, 1, &(the_floor.ftex));
	
	glTextureStorage2D(the_floor.ftex, 10, GL_RGB8, x, y);
	glTextureSubImage2D(the_floor.ftex, 0, 0, 0, x, y, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glGenerateTextureMipmap(the_floor.ftex);
	glTextureParameteri(the_floor.ftex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(the_floor.ftex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameterf(the_floor.ftex, GL_TEXTURE_MAX_ANISOTROPY, 8.0f);
	
	stbi_image_free(data);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glDeleteBuffers(1, &temp_load);
	
	if (GLAD_GL_ARB_bindless_texture) {
		the_floor.handle = glGetTextureHandleARB(the_floor.ftex);
		glMakeTextureHandleResidentARB(the_floor.handle);
	} else {
		glBindTextureUnit(0, the_floor.ftex);
	}
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	
	std::default_random_engine g;
	std::uniform_real_distribution<double> xd(-4.0, 4.0);
	std::uniform_real_distribution<double> vd(0.001f, 0.2f);
	
	for (size_t i = 0; i < num_balls; i++) {
		ball_pos[active_pos_buf][i] = glm::vec3((float)xd(g), 0.50000001f, (float)xd(g));
		ball_vel[i] = glm::vec3(vd(g), vd(g), vd(g));
	}
}

// computes the position of a ball with a given index in ball_pos and ball_arr
glm::vec3 calculate_pos(size_t i) {
	const float g_y = -0.01f;

	const float ball_radius = 0.5f;

	const float x_lim = 4.0;
	const float z_lim = 4.0;
	const float upper_y_lim = 4.0;

	const glm::vec3 Fg = glm::vec3(0.0f, g_y, 0.0f);
	
	glm::vec3 v = ball_vel[i];
	glm::vec3 x = ball_pos[active_pos_buf][i];
	
	// Our sim runs in steps.  This means that when the ball collides with the ground, it may not have the exact same velocity it did when it left the ground, because a full arc takes some non-whole amount of steps to simulate.  In order to not have the balls gain or lose energy on each bounce, we calculate exactly how much energy the ball should have at the time of the collision.

	if (x.y < ball_radius) {
		float x_prev = x.y - v.y;
		float x_d = ball_radius - x_prev;
		float v_part = v.y + (x_d / (x.y - x_prev)) * Fg.y;
		v = glm::vec3(v.x, -v_part, v.z);
		
		x.y += ball_radius - x.y;
	} else if (x.y > upper_y_lim) { // can't collide with more than one wall at the same time, save a cmp by using else if
		x.y -= x.y - upper_y_lim;
		v = glm::vec3(v.x, -v.y, v.z);
	} else {
		v = v + Fg / ball_mass;
		x = x + v;
	}

	// check for wall collision with all four invisible walls
	if (x.x < -x_lim) {
		x.x += -x_lim - x.x;
		v.x = -v.x;
	} else if (x.x > x_lim) {
		x.x -= x.x - x_lim;
		v.x = -v.x;
	}

	if (x.z < -z_lim) {
		x.z += -z_lim - x.z;
		v.z = -v.z;
	} else if (x.z > z_lim) {
		x.z -= x.z - z_lim;
		v.z = -v.z;
	}
	
	// check for collision with all other balls
	for (size_t other = 0; other < num_balls; other++) {
		if (other == i) {
			continue;
		}
		
		const glm::vec3 ov = ball_vel[other];
		const glm::vec3 ox = ball_pos[active_pos_buf][other];
		
		const glm::vec3 d = x - ox;
		const float ld = glm::length(d);
		const float intersect_amt = 2.0f * ball_radius - ld;
		if (intersect_amt > 0) {
			// collision occurred
			const float mag_d_2 = ld * ld;
			v = v - (glm::dot(v - ov, d) / mag_d_2) * d; // from Wikipedia
			x = x + d / 2.0f;
		}
	}

	ball_vel[i] = v;
	ball_pos[!active_pos_buf][i] = x;

	return x;
}

void render(shaderProgram& s) {
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	c.update(w);
	
	glUseProgramStages(s.pipeline, GL_VERTEX_SHADER_BIT, stages[vshdr]);
	glUseProgramStages(s.pipeline, GL_FRAGMENT_SHADER_BIT, stages[fshdr]);
	
	glm::mat4 viewMatrix = glm::lookAt(c.pos, c.pos + c.front, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(25.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

	glProgramUniformMatrix4fv(stages[vshdr], 1, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glProgramUniformMatrix4fv(stages[vshdr], 2, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	
	for (size_t i = 0; i < num_balls; i++) {
		glm::mat4 modelMatrix = glm::scale(glm::translate(glm::mat4(1.0f), calculate_pos(i)), glm::vec3(0.5f));
		glProgramUniformMatrix4fv(stages[vshdr], 0, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		
		glProgramUniform3fv(stages[fshdr], 0, 1, glm::value_ptr(ball_vel[i]));
		glBindVertexArray(ball.vao);
		glDrawElements(GL_TRIANGLES, ball.elems, GL_UNSIGNED_INT, nullptr);
	}
	active_pos_buf = !active_pos_buf; // swap position buffers

	glUseProgramStages(s.pipeline, GL_VERTEX_SHADER_BIT, stages[the_floor_vshdr]);
	glUseProgramStages(s.pipeline, GL_FRAGMENT_SHADER_BIT, stages[the_floor_fshdr]);
	
	if (GLAD_GL_ARB_bindless_texture) {
		glProgramUniformHandleui64ARB(stages[the_floor_fshdr], 0, the_floor.handle);
	}

	glm::mat4 floorModel = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));
	glProgramUniformMatrix4fv(stages[the_floor_vshdr], 0, 1, GL_FALSE, glm::value_ptr(floorModel));
	glProgramUniformMatrix4fv(stages[the_floor_vshdr], 1, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glProgramUniformMatrix4fv(stages[the_floor_vshdr], 2, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	glBindVertexArray(the_floor.vao);
	glDrawElements(GL_TRIANGLES, the_floor.elems, GL_UNSIGNED_INT, nullptr);
}

void run() {

	shaderProgram s;

	init(s);
	while (!glfwWindowShouldClose(w)) {
		if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(w, true);
		}
		
		render(s);

		glfwSwapBuffers(w);

		glfwPollEvents();
	}
}

int main(int argc, char **argv) {
	w = systemInit();
	run();
	systemDestroy(w);
}
