#include "model.h"
using namespace std;
using namespace load;

bool model::loadModel(const char* path) {
	Assimp::Importer imp;
	const aiScene* scene = imp.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals); // turn anything that isn't a triangle into a triangle, and make normals if they don't exist

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { // check if import failed and exit
		cerr << "Assimp scene import failed: " << imp.GetErrorString() << endl;
		return false;
	}

	processNode(scene->mRootNode, scene);

	for (int i = 0; i < meshList.size(); i++) {
		bufferGroup dummy{ 0, 0, 0, 0 };
		vboList.push_back(dummy);
		vaoList.push_back(0);
	}

	sendVertexData(); // send data to GPU

	return true;
}

bool model::loadTexture(GLenum type, string path, GLint binding) { // type of texture, path to texture, and sampler to bind the texture to.
	if (!vboList.size()) {
		cerr << "WARNING: Loading texture even though model data hasn't been loaded yet." << endl; // check the first mesh, if it doesn't have texture coords it's likely the rest of the model doesn't either
	}

	limage::image temp;
	return temp.loadImage(type, path, binding);
}

bool model::loadTextureArray(GLenum type, string* pathList, int numTextures, GLint binding) {
	if (!vboList.size()) {
		cerr << "WARNING: Loading texture even though model data hasn't been loaded yet." << endl;
	}

	limage::image tempCube;
	return tempCube.loadImageArray(type, pathList, numTextures, binding);
}

void model::sendVertexData() {
	for (int i = 0; i < meshList.size(); i++) {
		glCreateVertexArrays(1, &(vaoList[i]));

		glBindVertexArray(vaoList[i]);

		glCreateBuffers(1, &(vboList[i].pos)); // model definitely has position vertices

		glBindBuffer(GL_ARRAY_BUFFER, vboList[i].pos);

		glNamedBufferStorage(vboList[i].pos, sizeof(pt) * meshList[i].pList.size(), meshList[i].pList.data(), 0);

		glEnableVertexAttribArray(vPosition);
		glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, sizeof(pt), (GLvoid*)0);

		glEnableVertexAttribArray(vNormal);
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, sizeof(pt), (GLvoid*)sizeof(glm::vec3)); // normals stored with vertices

		if (meshList[i].texList.size() == 0) { // no texture coords mapped, so map everything to 0
			glVertexAttrib2f(vTexCoord, 0.0f, 0.0f);
		}
		else { // model has texcoords, send them too
			glCreateBuffers(1, &(vboList[i].uv));
			glBindBuffer(GL_ARRAY_BUFFER, vboList[i].uv);

			glNamedBufferStorage(vboList[i].uv, sizeof(glm::vec2) * meshList[i].texList.size(), meshList[i].texList.data(), 0);

			glEnableVertexAttribArray(vTexCoord);
			glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0); // should this be done only if a texture is loaded?
		}

		glCreateBuffers(1, &(vboList[i].elem)); // model definitely has elements
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboList[i].elem);
		glNamedBufferStorage(vboList[i].elem, sizeof(unsigned short) * meshList[i].elemList.size(), meshList[i].elemList.data(), 0);
	}

	// TEST SPACE for adding random stuff to shaders, note that this is executed per model when vertex data is sent to GPU.
	
	//GLuint temp;
	//glCreateBuffers(1, &temp);
	//glBindBuffer(GL_TEXTURE_BUFFER, temp);
	//glNamedBufferStorage(temp, sizeof(glm::vec3) * meshList[0].pList.size(), meshList[0].pList.data(),0);

	//GLuint tex;
	//glCreateTextures(GL_TEXTURE_BUFFER, 1, &tex);
	//glTextureBuffer(tex, GL_RGB32F, temp); // attach a texture to a buffer.
	//
	//glBindTextureUnit(2, tex);

	// END TEST SPACE
}

void model::draw(int color) {
	for (int i = 0; i < meshList.size(); i++) {
		glm::vec4 currColor = meshList[i].color;
		if (color) {
			glVertexAttrib4f(vColor, currColor.r, currColor.g, currColor.b, currColor.a);
		}

		glBindVertexArray(vaoList[i]);
		glDrawElements(GL_TRIANGLES, (GLsizei)meshList[i].elemList.size(), GL_UNSIGNED_SHORT, (GLvoid*)0);
	}
}

// recursively visit nodes, not my code
bool model::processNode(aiNode* node, const aiScene* scene) {
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* meshtemp = scene->mMeshes[node->mMeshes[i]];
		meshList.push_back(processMesh(meshtemp, scene));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene); // process children
	}

	return true;
}

mesh model::processMesh(aiMesh* inMesh, const aiScene* scene) {
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

model::~model() {
	if (vaoList.size() > 0) {
		glDeleteVertexArrays((GLsizei)vaoList.size(), vaoList.data());
	}

	if (vboList.size() > 0) {
		for (auto i = vboList.begin(); i != vboList.end(); i++) { // if a buffer isn't actually bound when deleted, glDeleteBuffers silently ignores it.
			glDeleteBuffers(1, &(i->pos));
			glDeleteBuffers(1, &(i->norm));
			glDeleteBuffers(1, &(i->uv));
			glDeleteBuffers(1, &(i->elem));
		}
	}
}