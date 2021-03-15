#include "vloader.h"
using namespace std;

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
	vector<glm::vec2> uvList;
	vector<unsigned short> elemList;
	glm::vec4 color;

	for (unsigned int i = 0; i < inMesh->mNumVertices; i++) { // extract position information
		glm::vec3 position;
		glm::vec3 normal;

		position.x = inMesh->mVertices[i].x;
		position.y = inMesh->mVertices[i].y;
		position.z = inMesh->mVertices[i].z;

		normal.x = inMesh->mNormals[i].x;
		normal.y = inMesh->mNormals[i].y;
		normal.z = inMesh->mNormals[i].z;

		// don't know about this rn and don't care rn
		//if (inMesh->mColors != nullptr) { // if colors are available, add them
		//	color.r = inMesh->mColors[i]->r;
		//	color.g = inMesh->mColors[i]->g;
		//	color.b = inMesh->mColors[i]->b;
		//	color.a = inMesh->mColors[i]->a;
		//}
		//else {
		//	color.r = color.g = color.b = color.a = 0.0f;
		//}

		vList.push_back(pt(position, normal));
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
			uvList.push_back(coord);
		}
	}

	// add only ambient color and opacity of the material - don't want to calculate lighting colors on the CPU when we can do it on the GPU
	aiColor3D ambColor;
	aiMaterial* mat = scene->mMaterials[inMesh->mMaterialIndex];
	int model;
	mat->Get(AI_MATKEY_SHADING_MODEL, model);
	if (model == aiShadingMode_Phong) {
		mat->Get(AI_MATKEY_COLOR_DIFFUSE, ambColor);
	}
	else if (model == aiShadingMode_Gouraud) {
		mat->Get(AI_MATKEY_COLOR_SPECULAR, ambColor);
	}
	else {
		cerr << "Unknown shading model!" << endl;
	}

	float opacity = 1.0f;
	mat->Get(AI_MATKEY_OPACITY, opacity);

	color.r = ambColor.r;
	color.g = ambColor.g;
	color.b = ambColor.b;
	color.a = opacity;

	return mesh(vList, uvList, elemList, color);
}