#include "camera.h"
#include <iostream>

bool first = true;
double mscale = 0.05; // movement speed
double lscale = 0.05; // look speed

camera::camera(GLFWwindow* window, glm::vec3 inPos) : pos(inPos), front(glm::vec3(0.0f, 0.0f, -1.0f)), prevX(0.0), prevY(0.0), hangle(-90.0f), vangle(0.0f) {
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void camera::update(GLFWwindow *window) {

	double x, y;
	glfwGetCursorPos(window, &x, &y);

	if (first) { // prevent jumping to weird screen coords when mouse first comes into view
		prevX = x;
		prevY = y;
		first = false;
	}

	// target is 1 away from pos, always.  This makes math easier (hypotenuse = 1)

	double xoff = x - prevX;
	double yoff = prevY - y;

	prevX = x;
	prevY = y;

	hangle += (float)(xoff * lscale);
	vangle += (float)(yoff * lscale);

	// sine and cosine functions affect more than one axis

	glm::vec3 dir;
	dir.x = cosf(glm::radians(vangle)) * cosf(glm::radians(hangle));
	dir.y = sinf(glm::radians(vangle));
	dir.z = cosf(glm::radians(vangle)) * sinf(glm::radians(hangle));

	front = glm::normalize(dir);
	
	glm::vec3 updir = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 rdir = glm::cross(updir, front);

	// move camera according to user input
	// callback is annoying since it only triggers when key is updated, this is kinda cleaner and I don't really care if I miss a single keypress or two
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		pos += glm::vec3((float)mscale) * front; // can't multiply a scalar with a vector here...
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		pos -= glm::vec3((float)mscale) * front;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		pos += glm::vec3((float)mscale) * rdir;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		pos -= glm::vec3((float)mscale) * rdir;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		pos += glm::vec3((float)mscale) * updir;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		pos -= glm::vec3((float)mscale) * updir;
	}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) { // reset camera to original location
		pos = glm::vec3(0.0f);
		hangle = -90.0f;
		vangle = 0.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		mscale = 0.5;
	}
}
