#pragma once
#include "stb_image.h"

#include <iostream>
#include <string>

#include <unordered_map>
#include <vector>

#include "glad.h"

namespace iload {

	// structs are spread out since smushing them together is more confusing and probably not that much faster
	
	struct texCacheInfo { // texture cache entry, containing texture information
		GLenum type;
		const int width, height, depth;
		unsigned char* data;
	};

	struct texHandle { // texture handle for passing to the GPU
		GLenum type;
		GLuint handle; // OpenGL texture handle
	};

	struct tpackage { // a struct to pass to image loading threads.
		int tid;
		std::string path;
		unsigned char** data; // record data, and dimensions
		int* width;
		int* height;
	};

	class iloader {
	public:
		// architecture is a little weird here.  lastLoad is the most recent set of textures, put here to be read out by the user.  unnecessarily complicated - could just return a pointer to the texture cache and have people read from there?
		std::vector<texCacheInfo> lastLoad;
		iloader(GLenum type, std::string path);
		iloader(GLenum type, std::string* pathList, int numTextures);
	};
}
