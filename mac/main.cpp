#include "header.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraDown = glm::vec3(0.0f, -1.0f, 0.0f);

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    
    float cameraSpeed = 0.5f; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        cameraPos += cameraSpeed * cameraUp;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        cameraPos -= cameraSpeed * cameraUp;
    }
    
}

int main(int argc, char **argv) {
    //NOTE: a window context has to be created before OpenGL can do any work at all!
    int height = 1200, width = 1200;
    
    //start GLFW section for actually drawing things
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); //set the opengl version to 3.3 (version used by doc)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //necesary on osx for some reason?
    cout << "Creating an OpenGL window..." << endl;

    //create a window object to hold all the windowing data
    GLFWwindow* window = glfwCreateWindow(height, width, "OpenGL", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create a window..." << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glViewport(0, 0, height, width); //set the rendering dimensions of the window

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //make a callback to be called every time the window size changes
    //enable the z-buffer
    glEnable(GL_DEPTH_TEST);
    cout << "Window created.\n\n";
    
    //figure out what we're running on and what version of OpenGL is supported
    cout << "Manufacturer: " << glGetString(GL_VENDOR) << endl;
    cout << "Version: " << glGetString(GL_VERSION) << ", Shader version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
    cout << "Hardware: " << glGetString(GL_RENDERER) << "\n\n";

    //next part is to compile the shaders!
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    string vertexShaderStr = readFile("shader/vertexShader.glsl"); //both shaders should be in the same directory
    string fragmentShaderStr = readFile("shader/fragmentShader.glsl");

    if (vertexShaderStr.compare("") == 0 || fragmentShaderStr.compare("") == 0) {
        cout << "No shader source files found." << endl;
        return -1;
    }

    //convert to a C string
    const char *vertexShaderSrc = vertexShaderStr.c_str();
    const char *fragmentShaderSrc = fragmentShaderStr.c_str();
    
    cout << "compiling shaders...";
    
    glShaderSource(vertexShader, 1, &vertexShaderSrc, NULL);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, NULL);

    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);

    char infoLog[512];
    //compile both and print errors if failed
    int vertexSuccess;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexSuccess);
    if (!vertexSuccess) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "vertex compilation failed!\n" << infoLog << endl;
        return -1;
    }

    int fragmentSuccess;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentSuccess);
    if (!fragmentSuccess) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "fragment compilation failed!\n" << infoLog << endl;
        return -1;
    }

    //create and link the final shader program
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    //check for link failure
    int finalSuccess;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &finalSuccess);
    if (!finalSuccess) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "shader linking failed!" << infoLog << endl;
        return -1;
    }
    
    glUseProgram(shaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    cout << " shaders compiled and linked." << endl;
    
    //all the proper vertices for a cube
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
//    unsigned int indices[] = {
//        0, 1, 3, // first triangle
//        1, 2, 3,  // second triangle
//    };
    
    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    
    unsigned int VBO[10];
    unsigned int VAO[10];
    //unsigned int EBO[10];
    
    //glGenBuffers(10, EBO);
    
    glGenBuffers(10, VBO);
    glGenVertexArrays(10, VAO);
    
    //bind VAO and send it data
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    //tell OpenGL how to parse vertex data
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    stbi_set_flip_vertically_on_load(true);
    
    //get texture info from parsing the png
    int texWidth, texHeight, numChannels;
    unsigned char *data = stbi_load("art/grass_texture.png", &texWidth, &texHeight, &numChannels, 0);
    if (!data) {
        cout << "no grass texture found." << endl;
        return -1;
    }
    
    int texWidthFace, texHeightFace, numChannelsFace;
    unsigned char *dataFace = stbi_load("art/awesomeface.png", &texWidthFace, &texHeightFace, &numChannelsFace, 0);
    if (!dataFace) {
        cout << "no face texture found." << endl;
        return -1;
    }
    
    unsigned int texture[10];
    glGenTextures(10, texture);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    
    //set textures to repeat on overflow
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    //use mipmaps when zooming in, nearest meighbor when zooming out
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    //generate an image with the previously loaded texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    //generate a mipmap as well
    glGenerateMipmap(GL_TEXTURE_2D);
    
    stbi_image_free(data); //free image data after it's been loaded
    
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    
    //set textures to repeat on overflow
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    //use mipmaps when zooming in, nearest meighbor when zooming out
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidthFace, texHeightFace, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataFace);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    stbi_image_free(dataFace);
    
    glUniform1i(glGetUniformLocation(shaderProgram, "texOne"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "texTwo"), 1);
    
    //wireframe mode by default
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    //projection matrix for perspective
    //set outside the render loop because it rarely changes

    
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //black background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture[1]); //bind and apply textures
        
        glUseProgram(shaderProgram);
        
        glm::mat4 view = glm::lookAt(cameraPos,
                                     cameraPos + cameraFront,
                                     cameraUp); //this has to point up to create the right vector to make the camera directions!
        
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), (float) width / (float) height, 0.1f, 100.0f);
        
        unsigned int viewLoc  = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc  = glGetUniformLocation(shaderProgram, "projection");
        
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        
        glBindVertexArray(VAO[0]);
        
        for (int i = 1; i <= 10; i++) {
            glm::mat4 model;
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle) * (float) glfwGetTime(), glm::vec3(1.0f, 0.3f, 0.5f));
            unsigned int modLoc = glGetUniformLocation(shaderProgram, "model");
            glUniformMatrix4fv(modLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        
        //glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(float), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    
    glDeleteBuffers(10, VBO);
    glDeleteBuffers(10, VAO);
    //delete EBO as well
    glfwTerminate();
    return 0;
}
