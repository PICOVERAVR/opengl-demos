#define STB_IMAGE_IMPLEMENTATION
#include "image.h"

#include <thread>

using namespace std;
using namespace limage;

std::unordered_map<std::string, texCacheInfo> texCache; // NOTE: texture cache persists over the lifetime of the program. texture data needs to be freed by stbi_image_free *and* delete[] on data members if we decide to free it.
bool texEnable = false; // enable/disable texture caching

// returns texture handle
void image::load(texInfo &t, GLint binding) {

	if (binding == -1) {
		cerr << "Invalid binding!" << endl;
		throw "gl error";
	}

	int width, height, numChannels;
	unsigned char* loadData;

	if (!texCache.count(t.path)) {
		loadData = stbi_load(t.path.c_str(), &width, &height, &numChannels, 0);
		if (loadData == nullptr) {
			cerr << "ERROR: image " << t.path << " load failed!" << endl;
			throw "file error";
		}
		std::pair<std::string, texCacheInfo> add(t.path, limage::texCacheInfo{ t.type, width, height, 0 });
		texCache.emplace(add);
	}
	else {
		limage::texCacheInfo temp = texCache.at(t.path);
		width = temp.width;
		height = temp.height;
		loadData = temp.data;
	}
	
	if (!sendImage(t, width, height, loadData, binding)) {
		cerr << "ERROR: OpenGL loading failed!" << endl;
		throw "gl error";
	}

	stbi_image_free(loadData);
}

bool image::sendImage(texInfo& t, const int width, const int height, const unsigned char* data, GLint binding) {

	GLuint tex;
	glCreateTextures(t.type, 1, &tex);
	t.handle = tex;

	switch (t.type) {
	case GL_TEXTURE_2D:

		glTextureStorage2D(tex, 8, GL_RGB8, width, height);
		
		GLuint loadBuffer;
		glCreateBuffers(1, &loadBuffer);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, loadBuffer);

		glNamedBufferStorage(loadBuffer, (GLsizeiptr)width * (GLsizeiptr)height * 3, data, 0);
		
		glTextureSubImage2D(tex, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0); // unbind buffer so data can be deleted
		glInvalidateBufferData(loadBuffer); // tell OpenGL data isn't needed anymore
		glDeleteBuffers(1, &(loadBuffer)); // delete buffer name

		cout << "binding 2D image to texture unit " << binding << endl;

		break;
	default:
		cerr << "Unknown texture type!" << endl;
		return false;
	}
	
	glBindTextureUnit(binding, tex);

	//GLuint tsamp;
	//glCreateSamplers(1, &tsamp);
	//glBindSampler(binding, tsamp);

	//glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateTextureMipmap(tex);
	return true;
}

// NOTE: originally, threadFunc and loadImageArray were written using a windows threading library.  The code has been re-written using standard C++ threads, but keeps the original style.

static int threadFunc(tpackage* x) {
    int width, height, numChannels;
    *(x->data) = stbi_load(x->path.c_str(), &width, &height, &numChannels, 0); // load first face and use the returned dimensions to set up the texture, and input the rest of the textures later
    *(x->width) = width;
    *(x->height) = height;
    return (x->data == nullptr); // return error if error occurred
}

// this is threaded since loading a lot of high-res textures in a cubemap can take a long time
void image::loadArray(texInfo* t, int numTextures, GLint binding) {

	if (binding == -1) {
		cerr << "Invalid binding!" << endl;
		throw "gl error";
	}

	if (numTextures <= 0) {
		throw "stupid error";
	}

	tpackage* threadDataList = new tpackage[numTextures];
	std::thread threadList[numTextures];
	unsigned char** dataList = new unsigned char* [numTextures];
	int* width = new int;
	int* height = new int;
	
	if (texCache.count(t[0].path) && texEnable) { // if one texture is in the cache, the others are too

		for (int i = 0; i < numTextures; i++) {
			limage::texCacheInfo temp = texCache.at(t[i].path);
			*width = temp.width;
			*height = temp.height;
			dataList[i] = temp.data;
		}
	}
	else {
		
		for (int i = 0; i < numTextures; i++) { // we can get a pretty nice speedup by loading each image in a seperate thread
			threadDataList[i].tid = i;
			threadDataList[i].path = t[i].path;
			threadDataList[i].data = dataList + i;
			threadDataList[i].width = width;
			threadDataList[i].height = height;

			threadList[i] = std::thread(threadFunc, threadDataList + i);
			//threadList[i] = CreateThread(nullptr, 0, threadFunc, (LPVOID)(threadDataList + i), 0, nullptr);
		}

		// int retReason = WaitForMultipleObjects(numTextures, threadList, TRUE, INFINITE);
		for (int i = 0; i < numTextures; i++) {
        	threadList[i].join();
    	}
		
		for (int i = 0; i < numTextures; i++) {
			std::pair<std::string, texCacheInfo> temp(t[i].path, limage::texCacheInfo{ t[0].type, *width, *height, 0, dataList[i]});
			texCache.emplace(temp);
		}
	}

	sendImageArray(t, numTextures, *width, *height, dataList, binding);

	delete[] threadDataList;
	//delete[] dataList;
	delete width;
	delete height;
}

bool image::sendImageArray(texInfo* t, int numTextures, const int width, const int height, unsigned char** dataList, GLint binding) {

	GLuint tex;
	glCreateTextures(t->type, 1, &tex);

	switch (t[0].type) { // all six textures are the same type
	case GL_TEXTURE_CUBE_MAP:

		glTextureStorage2D(tex, 8, GL_RGB8, width, height);

		for (int i = 0; i < numTextures; i++) {
			// keeping the serial version here just in case
			// width, height, numChannels aren't used here since all cubemap images have to be of the same dimensions
			//unsigned char* loadData = stbi_load(pathList[i], &width, &height, &numChannels, 0); // load next face, thread this
			//if (loadData == nullptr) {
			//	cerr << "ERROR: image load " << i << " failed!" << endl;
			//	return false;
			//}

			//glTextureSubImage3D(newTex, 0, 0, 0, i, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, loadData);
			//stbi_image_free(loadData);

			t[i].handle = tex;

			GLuint loadBuffer;
			glCreateBuffers(1, &loadBuffer);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, loadBuffer);

			glNamedBufferStorage(loadBuffer, (GLsizeiptr)width * (GLsizeiptr)height * 3, dataList[i], 0);

			glTextureSubImage3D(tex, 0, 0, 0, i, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0); // unbind buffer so data can be deleted
			glInvalidateBufferData(loadBuffer); // tell OpenGL data isn't needed anymore
			glDeleteBuffers(1, &(loadBuffer)); // delete buffer name
		}

		//for (int i = 0; i < 6; i++) {
			//stbi_image_free(dataList[i]); // free image data
		//}

		cout << "binding cube map to texture unit " << binding << endl;

		break;
	default:
		cerr << "Unknown texture array type!" << endl;
		return false;
	}

	glBindTextureUnit(binding, tex);

	//GLuint tsamp;
	//glCreateSamplers(1, &tsamp);
	//glBindSampler(binding, tsamp);

	glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateTextureMipmap(tex);

	glTextureParameterf(tex, GL_TEXTURE_MAX_ANISOTROPY, 16.0f);

	return true;
}
