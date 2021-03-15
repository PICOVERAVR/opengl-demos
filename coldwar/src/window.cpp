#include "window.h"

#define _DEBUG

static bool glfwCreate() {

	if (glfwInit() == GLFW_FALSE) {
		std::cerr << "ERROR: GLFW initialization failed!" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // mandate that glfw uses OpenGL version 4.6
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); // don't have the book for this, but it's the most recent version.  Whatever.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // dump backwards-compatible functions we're not going to use
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE); // dump deprecated functions in OpenGL 4.6s
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE); // not fullscreen but dump close icon stuff

#ifdef _DEBUG
	std::cout << "Debug info enabled." << std::endl;
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // get more debug information
#else
	std::cout << "Debug info disabled." << std::endl;
	glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_TRUE); // get no debug information
#endif

	return true;
}

// called when screen resolution is changed
void framebufferSizeCallback(GLFWwindow* w, int width, int height) {
	glViewport(0, 0, width, height);
}

// callback for OpenGL debug information
// GLAPIENTRY tells the compiler to use the stdcall calling convention instead of whatever is default
// APIENTRY also works here
void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::cerr << " ~ " << message << std::endl;

	if (severity == GL_DEBUG_SEVERITY_HIGH) {
		std::cerr << " ~ " << "Severe error, aborting now." << std::endl;
		abort();
	}
}

void interpretFramebufferStatus(GLenum error) {
	switch (error) {
	case GL_FRAMEBUFFER_COMPLETE:
		std::cout << "Framebuffer is complete." << std::endl;
		break;
	case GL_FRAMEBUFFER_UNDEFINED:
		std::cout << "GL_FRAMEBUFFER_UNDEFINED: the specified framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist." << std::endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: any of the framebuffer attachment points are framebuffer incomplete." << std::endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: the framebuffer does not have at least one image attached to it." << std::endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		std::cout << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAW_BUFFERi." << std::endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		std::cout << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER." << std::endl;
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		std::cout << "GL_FRAMEBUFFER_UNSUPPORTED: the combination of internal formats of the attached images violates an implementation - dependent set of restrictions." << std::endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		std::cout << "the value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers; if the value of GL_TEXTURE_SAMPLES is the not same for all attached textures; or , if the attached images are a mix of renderbuffersand textures, " \
			"the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES. OR the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures; or , if the attached images are a mix of renderbuffersand textures, the" \
			" value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached textures." << std::endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		std::cout << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: any framebuffer attachment is layered, and any populated attachment is not layered, or if all populated color attachments are not from textures of the same target." << std::endl;
		break;
	default:
		std::cerr << "Unknown enum." << std::endl;
		break;
	}
}

GLFWwindow* systemInit() {
	if (!glfwCreate()) {
		return nullptr;
	}

	GLFWwindow* window;
	window = glfwCreateWindow(screenWidth, screenHeight, "OpenGLHQ", nullptr, nullptr);
	glfwSetWindowTitle(window, "OpenGL Window");

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

#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(debugCallback, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif

	return window;
}

bool systemDestroy(GLFWwindow *w) {
	glfwDestroyWindow(w);
	glfwTerminate();
	return true;
}
