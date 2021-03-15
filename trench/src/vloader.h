#pragma once

#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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

	class vloader {
	public:
		vloader(std::string path);
		std::vector<mesh> meshList;
	private:
		bool processNode(aiNode* node, const aiScene* scene);
		mesh processMesh(aiMesh* inMesh, const aiScene* scene);
	};

