/*
	this class encapsulates loading and drawing things because it gets really tiresome when more than one type of object is drawn.
*/

#pragma once

#include <vector>
#include <iostream>
#include <string>

#include <glm/glm.hpp>

#include "vloader.h" // needs to be before image since image includes windows.h which defines min and max
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

	struct meshlite {
		glm::vec4 color; // color of mesh
		int psize; // size of various elements, psize kept because I can replace it later
		int tsize;
		int esize;
		int eoffset; // offset from the beginning of the buffer allocated for all meshes in the model
	};

	class model {
	public:
		model() : vaoList(), vboList(), texList() { }
		~model();

		model& operator=(const model& other);

		bool load(std::vector<mesh>&);

		bool loadTexture(limage::texInfo &t, GLint binding);
		bool loadTextureArray(limage::texInfo* t, int numTextures, GLint binding); // load more than one texture at once

		void draw(const int color) const; // color indicates if the model should use internal colors when drawing, sometimes helpful to not be colored
		void drawInstanced(const int color, const int count) const;

	private:
		std::vector<GLuint> vaoList;
		std::vector<bufferGroup> vboList; // one list of buffers per mesh
		std::vector<limage::texInfo> texList;

		std::vector<meshlite> meshInfo; // enough to draw a mesh
	};
}
