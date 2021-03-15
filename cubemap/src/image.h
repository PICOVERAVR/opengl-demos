#pragma once

/*

	normally this wouldn't be a seperate file but I want to load multiple images at a time here, and loading each image in a seperate thread makes things a lot faster
	need to put all this in a class in order to facilitate actually deleting image memory once done

*/

#include "stb_image.h"

#include <iostream>
#include <vector>
#include <string>

#include "glad.h"

namespace limage {

	struct tpackage {
		int tid;
		std::string path;
		unsigned char** data; // record data, and dimensions
		int* width;
		int* height;
	};

	class image {
	public:
		bool loadImage(GLenum type, const std::string path, GLint binding);
		bool loadImageArray(GLenum type, const std::string* pathList, int numTextures, GLint binding);
	private:
		void sendImage(GLenum type, int width, int height, const unsigned char* data, GLint binding);
		void sendImageArray(GLenum type, int width, int height, unsigned char** dataList, GLint binding);
	};

}
