#include "header.h"

int main(int argc, char **argv) {
    int height = 3200, width = 2400;
    
    //start GLFW section for actually drawing things
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); //set the opengl version to 3.3 (version used by doc)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //necesary on osx for some reason?

    //create a window object to hold all the windowing data
    GLFWwindow* window = glfwCreateWindow(height, width, "OpenGL", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create a window..." << endl;
        glfwTerminate();
        return -1;
    }
    cout << "Window created.\n\n";
	
 	glfwMakeContextCurrent(window);
	glViewport(0, 0, height, width); //set the rendering dimensions of the window

    //figure out what we're running on and what version of OpenGL is supported
    cout << "Manufacturer: " << glGetString(GL_VENDOR) << endl;
    cout << "Version: " << glGetString(GL_VERSION) << ", Shader version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
    cout << "Hardware: " << glGetString(GL_RENDERER) << "\n\n";

    glfwTerminate();
    return 0;
}
