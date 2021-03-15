#pragma once

/*

	normally this wouldn't be a seperate file but I want to load multiple images at a time here, and loading each image in a seperate thread makes things a lot faster
	need to put all this in a class in order to facilitate actually deleting image memory once done

*/

#include "stb_image.h"

#include <iostream>
#include <string>

#include <unordered_map>

#include "glad.h"

namespace limage {

	struct texInfo {
		GLenum type;
		GLuint handle;

		std::string path;
	};

	struct texCacheInfo {
		GLenum type;
		const int width, height, depth;
		unsigned char* data;
	};

	struct tpackage {
		int tid;
		std::string path;
		unsigned char** data; // record data, and dimensions
		int* width;
		int* height;
	};

	class image {
	public:
		void load(texInfo& t, GLint binding);
		void loadArray(texInfo* t, int numTextures, GLint binding);

	private:
		bool sendImage(texInfo &t, const int width, const int height, const unsigned char* data, GLint binding);
		bool sendImageArray(texInfo* t, int numTextures, const int width, const int height, unsigned char** dataList, GLint binding); // probably need more const here
	};
}
