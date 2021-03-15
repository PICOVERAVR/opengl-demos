#pragma once

//#define _CRTDBG_MAP_ALLOC // enable to output a report of memory leaks at the end of execution
//#include <cstdlib>
//#include <crtdbg.h>

#include <iostream>
#include <string>

#include "window.h" // this has to be included first, since it uses GLAD to set up the GLFW3 headers.
#include "program.h"
#include "camera.h"

#include "vloader.h"
#include "stb_image.h"

#include <glm/glm.hpp> // header-only lib for matrix and vector math
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // extract floats from matrix

#include <GLFW/glfw3.h>
