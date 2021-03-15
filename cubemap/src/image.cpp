#define STB_IMAGE_IMPLEMENTATION
#include "image.h"

#include <thread>

using namespace std;
using namespace limage;

//std::vector<texture> texCache;
GLuint texUnitAvail = 0; // next available texture unit for use - target can be fetched using glGetTextureParameteriv(<textureid>, GL_TEXTURE_TARGET, &target);

bool image::loadImage(GLenum type, const std::string path, GLint binding) {

	if (binding == -1) {
		cerr << "Could not locate uniform to bind to!" << endl;
		return false;
	}

	int width, height, numChannels;
	unsigned char* loadData = stbi_load(path.c_str(), &width, &height, &numChannels, 0); // load face
	if (loadData == nullptr) {
		cerr << "ERROR: image " << path << " load failed!" << endl;
		return false;
	}

	sendImage(type, width, height, loadData, binding);

	stbi_image_free(loadData);

	return true;
}

void image::sendImage(GLenum type, int width, int height, const unsigned char* data, GLint binding) {

	GLuint tex;
	glCreateTextures(type, 1, &tex);

	switch (type) {
	case GL_TEXTURE_2D:

		glTextureStorage2D(tex, 1, GL_RGB8, width, height);
		
		GLuint loadBuffer;
		glCreateBuffers(1, &loadBuffer);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, loadBuffer);

		glNamedBufferStorage(loadBuffer, (GLsizeiptr)width * (GLsizeiptr)height * 3, data, 0);
		
		glTextureSubImage2D(tex, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0); // unbind buffer so data can be deleted
		glInvalidateBufferData(loadBuffer); // tell OpenGL data isn't needed anymore
		glDeleteBuffers(1, &(loadBuffer)); // delete buffer name

		break;
	default:
		cerr << "Unknown texture type!" << endl;
		return;
	}
	
	//cout << "binding to 2D texture unit " << texUnitAvail[unit2D] << endl;
	
	glBindTextureUnit(binding, tex);

	GLuint tsamp;
	glCreateSamplers(1, &tsamp);
	glBindSampler(texUnitAvail, tsamp);

	glSamplerParameteri(tsamp, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(tsamp, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateTextureMipmap(tex); // could remove this or add mipmaps manually, idk

	//glSamplerParameterf(tsamp, GL_TEXTURE_MAX_ANISOTROPY, 16.0f); // add anisotropic filtering in order to mipmap different levels in different dimensions

	texUnitAvail++;
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
bool image::loadImageArray(GLenum type, const string* pathList, int numTextures, GLint binding) {

	if (binding == -1) {
		cerr << "Could not locate uniform to bind to!" << endl;
		return false;
	}

	tpackage* threadDataList = new tpackage[6];
	std::thread threadList[6];
	unsigned char** dataList = new unsigned char* [6];
	int* width = new int;
	int* height = new int;

	for (int i = 0; i < numTextures; i++) { // we can get a pretty nice speedup by loading each image in a seperate thread
		threadDataList[i].tid = i;
		threadDataList[i].path = pathList[i];
		threadDataList[i].data = dataList + i;
		threadDataList[i].width = width;
		threadDataList[i].height = height;

		threadList[i] = std::thread(threadFunc, threadDataList + i);
		// threadList[i] = CreateThread(nullptr, 0, threadFunc, (LPVOID)(threadDataList + i), 0, nullptr);
	}
	
	// int retReason = WaitForMultipleObjects(5, threadList, TRUE, INFINITE);
	for (int i = 0; i < numTextures; i++) {
		threadList[i].join();
	}

	sendImageArray(type, *width, *height, dataList, binding);

	// free stuff
	
	return true; // fix later
}

void image::sendImageArray(GLenum type, int width, int height, unsigned char** dataList, GLint binding) {

	GLuint tex;
	glCreateTextures(type, 1, &tex);

	switch (type) {
	case GL_TEXTURE_CUBE_MAP:

		glTextureStorage2D(tex, 1, GL_RGB8, width, height);

		for (int i = 0; i < 6; i++) {
			// keeping the serial version here just in case
			// width, height, numChannels aren't used here since all cubemap images have to be of the same dimensions
			//unsigned char* loadData = stbi_load(pathList[i], &width, &height, &numChannels, 0); // load next face, thread this
			//if (loadData == nullptr) {
			//	cerr << "ERROR: image load " << i << " failed!" << endl;
			//	return false;
			//}

			//glTextureSubImage3D(newTex, 0, 0, 0, i, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, loadData);
			//stbi_image_free(loadData);

			GLuint loadBuffer;
			glCreateBuffers(1, &loadBuffer);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, loadBuffer);

			glNamedBufferStorage(loadBuffer, (GLsizeiptr)width * (GLsizeiptr)height * 3, dataList[i], 0);

			glTextureSubImage3D(tex, 0, 0, 0, i, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0); // unbind buffer so data can be deleted
			glInvalidateBufferData(loadBuffer); // tell OpenGL data isn't needed anymore
			glDeleteBuffers(1, &(loadBuffer)); // delete buffer name

		}

		for (int i = 0; i < 6; i++) {
			stbi_image_free(dataList[i]); // free image data
		}

		break;
	default:
		cerr << "Unknown texture array type!" << endl;
		return;
	}

	//cout << "binding to cube texture unit " << texUnitAvail[unit3D] << endl;

	glBindTextureUnit(binding, tex);

	GLuint tsamp;
	glCreateSamplers(1, &tsamp);
	glBindSampler(texUnitAvail, tsamp);

	glSamplerParameteri(tsamp, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(tsamp, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateTextureMipmap(tex); // could remove this or add mipmaps manually, idk

	glSamplerParameterf(tsamp, GL_TEXTURE_MAX_ANISOTROPY, 16.0f);

	texUnitAvail++;
}
