#include "window.h"
using std::endl;
using std::cout;
using std::cerr;

// called when screen resolution is changed
static void framebufferSizeCallback(GLFWwindow* w, int width, int height) {
	glViewport(0, 0, width, height);
}

// callback for OpenGL debug information
// GLAPIENTRY tells the compiler to use the stdcall calling convention instead of whatever is default
// APIENTRY also works here
static void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	cerr << " ~ " << message << endl;

	if (severity == GL_DEBUG_SEVERITY_HIGH) {
		cerr << " ~ " << "Aborting now." << endl;
		abort();
	}
}

window::window() {
	if (glfwInit() == GLFW_FALSE) {
		throw "GLFW initialization error!";
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // mandate that glfw uses OpenGL version 4.6
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); // don't have the book for this, but it's the most recent version.  Whatever.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // dump backwards-compatible functions we're not going to use
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE); // dump deprecated functions in OpenGL 4.6s
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE); // not fullscreen but dump close icon stuff

#ifndef NDEBUG
	cout << "Debug info enabled." << endl;
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // get more debug information
#else
	cout << "OpenGL errors suppressed." << endl;
	glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_TRUE); // get no debug information
#endif

	wptr = glfwCreateWindow(screenWidth, screenHeight, "Demo", nullptr, nullptr);
	if (!wptr) {
		glfwTerminate();
		throw "GLFW window creation error!";
	}
	
	glfwMakeContextCurrent(wptr);
	glfwSetFramebufferSizeCallback(wptr, framebufferSizeCallback);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		throw "Failed to initialize GLAD!";
	}

#ifndef NDEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(debugCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif
	
	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	cout << "Hardware: " << glGetString(GL_RENDERER) << endl;
	cout << "Manufacturer: " << glGetString(GL_VENDOR) << endl << endl;
}

window::~window() {
	glfwDestroyWindow(wptr);
	glfwTerminate();
}

// misc function for printing out more helpful framebuffer information
#ifdef NDEBUG
void interpretFramebufferStatus(GLenum error) { }
#else
void interpretFramebufferStatus(GLenum error) {
	switch (error) {
	case GL_FRAMEBUFFER_COMPLETE:
		cout << "Framebuffer complete." << endl;
		break;
	case GL_FRAMEBUFFER_UNDEFINED:
		cout << "GL_FRAMEBUFFER_UNDEFINED: the specified framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist." << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: any of the framebuffer attachment points are framebuffer incomplete." << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: the framebuffer does not have at least one image attached to it." << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		cout << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAW_BUFFERi." << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		cout << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER." << endl;
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		cout << "GL_FRAMEBUFFER_UNSUPPORTED: the combination of internal formats of the attached images violates an implementation - dependent set of restrictions." << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		cout << "the value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers; if the value of GL_TEXTURE_SAMPLES is the not same for all attached textures; or , if the attached images are a mix of renderbuffersand textures, " \
			"the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES. OR the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures; or , if the attached images are a mix of renderbuffersand textures, the" \
			" value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached textures." << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		cout << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: any framebuffer attachment is layered, and any populated attachment is not layered, or if all populated color attachments are not from textures of the same target." << endl;
		break;
	case 0:
		cerr << "An error occurred!" << endl;
		break;
	default:
		cerr << "Unknown enum 0x" << std::hex << error << std::dec << endl;
		break;
	}
}
#endif
