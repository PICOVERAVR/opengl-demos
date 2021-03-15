#include "vloader.h"
using namespace std;

namespace vload {

	vloader::vloader(std::string path) : meshList() {
		Assimp::Importer imp;
		const aiScene* scene = imp.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals); // turn anything that isn't a triangle into a triangle, and make normals if they don't exist

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { // check if import failed and exit
			cerr << "Assimp scene import failed: " << imp.GetErrorString() << endl;
			throw "file error";
		}

		processNode(scene->mRootNode, scene);
	}

	// recursively visit nodes, not my code
	bool vloader::processNode(aiNode* node, const aiScene* scene) {
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* meshtemp = scene->mMeshes[node->mMeshes[i]];
			meshList.push_back(processMesh(meshtemp, scene));
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene); // process children
		}

		return true;
	}

	mesh vloader::processMesh(aiMesh* inMesh, const aiScene* scene) {
		vector<pt> vList;
		vector<unsigned int> elemList;

		for (unsigned int i = 0; i < inMesh->mNumVertices; i++) { // extract position information
			glm::vec3 position;
			glm::vec3 normal;

			position.x = inMesh->mVertices[i].x;
			position.y = inMesh->mVertices[i].y;
			position.z = inMesh->mVertices[i].z;

			normal.x = inMesh->mNormals[i].x;
			normal.y = inMesh->mNormals[i].y;
			normal.z = inMesh->mNormals[i].z;
			pt temppoint = { position, normal };
			vList.push_back(temppoint);
		}

		for (unsigned int i = 0; i < inMesh->mNumFaces; i++) { // extract element information
			aiFace f = inMesh->mFaces[i];
			for (unsigned int j = 0; j < f.mNumIndices; j++) {
				elemList.push_back(f.mIndices[j]);
			}
		}

		if (!inMesh->mTextureCoords[0]) { // extract texture information, if available
			cout << "Mesh has no texture coordinates, not loading anything." << endl;
		}
		else {
			for (unsigned int i = 0; i < inMesh->mNumVertices; i++) {
				glm::vec2 coord;
				coord.s = inMesh->mTextureCoords[0][i].x;
				coord.t = inMesh->mTextureCoords[0][i].y;
				vList[i].texcoord = coord;
			}
		}
		
		return mesh(vList, elemList);
	}

}