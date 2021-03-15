# LinuxGL
This repo serves as a base for writing small OpenGL programs, and includes several components to speed development.  This repo is _not_ designed to serve as an engine of any kind, and I left out many things an engine would include.
 - program.cpp: collects and compiles GLSL programs together.
 - glad.c: a GLAD loader with the extensions I've used so far.
 - camera.cpp: an fps-style camera class for looking around.
 - vloader.cpp: a frontend for assimp.
 - window.cpp: a frontend for GLFW that only supports the most recent version of OpenGL.

# Dependancies
 - GLFW3: window management.
 - Assimp: loading 3D objects from basically any format.
 - GLAD: loading OpenGL functions at runtime.
 - stb_image: loading images from most formats.
 - glm: OpenGL math lib.
 
