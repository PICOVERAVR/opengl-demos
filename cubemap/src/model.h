/*
	this class encapsulates loading and drawing things because it gets really tiresome when more than one type of object is drawn.
*/

#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <iostream>
#include <string>

#include <glm/glm.hpp>

#include "image.h"

#include "glad.h"

namespace load {

	enum shaderPositions { // standard positions for stuff on GPU side
		vPosition,
		vNormal,
		vColor,
		vTexCoord,
		eCoord,
	};

	struct bufferGroup {
		GLuint pos;
		GLuint norm;
		GLuint elem;
		GLuint uv;
	};

	class pt {
	public:
		glm::vec3 pos;
		glm::vec3 normal;

		pt(glm::vec3 inPos, glm::vec3 inNormal) : pos(inPos), normal(inNormal) { }
	};

	class mesh {
	public:
		std::vector<pt> pList; // position coords
		std::vector<glm::vec2> texList; // texture coords
		std::vector<unsigned short> elemList; // elements to dictate what to render
		glm::vec4 color; // 1 color per model for now

		mesh(std::vector<pt> inPList, std::vector<glm::vec2> inTexList, std::vector<unsigned short> inElemList, glm::vec4 inColor) : pList(inPList), texList(inTexList), elemList(inElemList), color(inColor) { }
		mesh() { }
	};

	class model {
	public:
		model() : vaoList(), vboList() { }
		~model();

		std::vector<mesh> meshList; // >1 mesh totally possible for more complex models, leaving this public for fiddling

		bool loadModel(const char* path);
		bool loadTexture(GLenum type, std::string path, GLint binding);
		bool loadTextureArray(GLenum type, std::string* pathList, int numTextures, GLint binding); // load more than one texture at once

		void draw(int color); // color indicates if the model should use internal colors when drawing, sometimes helpful to not be colored

	private:
		std::vector<GLuint> vaoList;
		std::vector<bufferGroup> vboList; // one list of buffers per mesh
		
		bool processNode(aiNode*, const aiScene*); // process a node and add the mesh to the mesh list
		mesh processMesh(aiMesh*, const aiScene*); // process a mesh, adding position and normal coords to the respective vectors

		void sendVertexData();
	};
}
