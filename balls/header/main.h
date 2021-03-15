#pragma once

#include <iostream>
#include <string>
#include <random>

#include "window.h" // this has to be included first, since it uses GLAD to set up the GLFW3 headers.
#include "program.h"
#include "camera.h"

#include "vloader.h"
#include "stb_image.h"

#include <glm/glm.hpp> // header-only lib for matrix and vector math
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // extract floats from matrix

#include <GLFW/glfw3.h>
