#define STB_IMAGE_IMPLEMENTATION
#include "iloader.h"

#include <thread>

using namespace std;

namespace iload {
	
	std::unordered_map<std::string, texCacheInfo> texCache; // NOTE: texture cache persists over the lifetime of the program. texture data needs to be freed by stbi_image_free *and* delete[] on data members if we decide to free it.

	iloader::iloader(GLenum type, std::string path) : lastLoad() {

		int width, height, numChannels;
		unsigned char* loadData;

		if (!texCache.count(path)) {
			loadData = stbi_load(path.c_str(), &width, &height, &numChannels, 0);

			if (loadData == nullptr) {
				cerr << "ERROR: image " << path << " load failed!" << endl;
				throw "file error";
			}
			std::pair<std::string, texCacheInfo> add(path, texCacheInfo { type, width, height, 0, loadData });

			texCache.emplace(add);
		}
		else {
			texCacheInfo temp = texCache.at(path);
			width = temp.width;
			height = temp.height;
			loadData = temp.data;
		}

		lastLoad.push_back(texCacheInfo{ type, width, height, 0, loadData });
	}

	// function executed in parallel, no kind of locking needed since each thread gets one image to load.
	// NOTE: originally, threadFunc and loadImageArray were written using a windows threading library.  The code has been re-written using standard C++ threads, but keeps the original style.

	static int threadFunc(tpackage* x) {
		int width, height, numChannels;
		*(x->data) = stbi_load(x->path.c_str(), &width, &height, &numChannels, 0); // load first face and use the returned dimensions to set up the texture, and input the rest of the textures later
		*(x->width) = width;
		*(x->height) = height;
		return (x->data == nullptr); // return error if error occurred
	}

	iloader::iloader(GLenum type, std::string* paths, int numTextures) : lastLoad() {

		if (numTextures <= 0) {
			throw "stupid error";
		}

		tpackage* threadDataList = new tpackage[numTextures];
		std::thread threadList[numTextures];
		unsigned char** dataList = new unsigned char* [numTextures];
		int* width = new int;
		int* height = new int;

		if (texCache.count(paths[0])) { // if one texture is in the cache, the others are too

			for (int i = 0; i < numTextures; i++) {
				texCacheInfo temp = texCache.at(paths[i]);
				*width = temp.width;
				*height = temp.height;
				dataList[i] = temp.data;
			}
		}
		else {

			for (int i = 0; i < numTextures; i++) { // we can get a pretty nice speedup by loading each image in a seperate thread
				threadDataList[i].tid = i;
				threadDataList[i].path = paths[i];
				threadDataList[i].data = dataList + i;
				threadDataList[i].width = width;
				threadDataList[i].height = height;
				
				threadList[i] = std::thread(threadFunc, threadDataList + i);
				// threadList[i] = CreateThread(nullptr, 0, threadFunc, (LPVOID)(threadDataList + i), 0, nullptr);
			}

			// int retReason = WaitForMultipleObjects(numTextures, threadList, TRUE, INFINITE);
			for (int i = 0; i < numTextures; i++) {
		        threadList[i].join();
   			}

			for (int i = 0; i < numTextures; i++) {
				std::pair<std::string, texCacheInfo> temp(paths[i], texCacheInfo{ type, *width, *height, 0, dataList[i] });
				texCache.emplace(temp);
			}
		}

		for (int i = 0; i < numTextures; i++) {
			texCacheInfo tinfo = { type, *width, *height, 0, dataList[i] };
			lastLoad.push_back(tinfo);
		}

		delete[] threadDataList;
		//delete[] dataList;
		delete width;
		delete height;
	}
}
