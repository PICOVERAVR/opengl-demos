# OpenGL
Why didn't I start with learning Vulkan?
## Setup
- Set up Visual Studio, configure paths to point to stuff below
- Download files from [GLAD](https://glad.dav1d.de/), targeting OpenGL Core 4.6.
- Install CMake (command line or GUI, GUI is a lot easier)
- Build [Assimp](https://github.com/assimp/assimp) using CMake.
  - Keep PDBs, turn off test coverage
  - Build as release version, turn optimizations on.  This thing needs to be fast.
- Download [stbi_image](https://github.com/nothings/stb/blob/master/stb_image.h)
## Usage
Camera controls:
- `W`: move forward (in direction of where you're looking)
- `S`: move back
- `A`: move left
- `D`: move right
- `mouse`: look around
- `left shift`: move down
- `space`: move up
- `R`: reset camera to origin
- `1`: move faster
