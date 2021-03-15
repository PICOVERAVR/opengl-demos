#include "window.h"

static bool glfwCreate() {

	if (glfwInit() == GLFW_FALSE) {
		std::cerr << "ERROR: GLFW initialization failed!" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // mandate that glfw uses OpenGL version 4.6
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); // don't have the book for this, but it's the most recent version.  Whatever.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // dump backwards-compatible functions we're not going to use	

	glfwWindowHint(GLFW_SAMPLES, 4); // request multisampled framebuffer, note that this increases total memory consumption by ~2.8x
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // get more debug information

	return true;
}

// called when screen resolution is changed
void framebufferSizeCallback(GLFWwindow* w, int width, int height) {
	glViewport(0, 0, width, height);
}

// callback for OpenGL debug information
// GLAPIENTRY tells the compiler to use the stdcall calling convention instead of whatever is default
// APIENTRY also works here
// new, better way of detecting OpenGL errors - way more info!
void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::cerr << " ~ " << message << std::endl;

	if (severity == GL_DEBUG_SEVERITY_HIGH) {
		std::cerr << " ~ " << "Severe error, aborting now." << std::endl;
		abort();
	}
}

GLFWwindow* systemInit() {
	if (!glfwCreate()) {
		return nullptr;
	}

	GLFWwindow* window;
	window = glfwCreateWindow(screenWidth, screenHeight, "OpenGLHQ", nullptr, nullptr); // almost fullscreen, but can see other stuff when debugging also

	if (window == nullptr) {
		std::cerr << "ERROR: GLFW window creation failed!" << std::endl;
		glfwTerminate();
		return nullptr;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback); // register screen change callback
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { // load GLAD
		std::cerr << "ERROR: Failed to initialize GLAD!" << std::endl;
		return nullptr;
	}

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(debugCallback, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

	return window;
}

bool systemDestroy(GLFWwindow *w) {
	glfwDestroyWindow(w);
	glfwTerminate();
	return true;
}