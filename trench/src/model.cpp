#include "model.h"
using namespace std;
using namespace load;
using namespace limage;

bool model::load(std::vector<mesh>& meshList) { // vertices provided, upload them and load config data into the model
	for (int i = 0; i < meshList.size(); i++) {
		vboList.push_back(bufferGroup{ 0, 0, 0, 0 });
		vaoList.push_back(0);

		glCreateVertexArrays(1, &(vaoList[i]));

		glBindVertexArray(vaoList[i]);

		glCreateBuffers(1, &(vboList[i].pos)); // model definitely has position vertices
		glNamedBufferStorage(vboList[i].pos, sizeof(pt) * meshList[i].pList.size(), meshList[i].pList.data(), 0);
		
		glVertexArrayVertexBuffer(vaoList[i], vPosition, vboList[i].pos, 0, sizeof(pt));
		glVertexArrayVertexBuffer(vaoList[i], vNormal, vboList[i].pos, sizeof(glm::vec3), sizeof(pt));

		glEnableVertexArrayAttrib(vaoList[i], vPosition);
		glVertexArrayAttribBinding(vaoList[i], vPosition, 0);
		glVertexArrayAttribFormat(vaoList[i], vPosition, 3, GL_FLOAT, GL_FALSE, 0);

		glEnableVertexArrayAttrib(vaoList[i], vNormal);
		glVertexArrayAttribBinding(vaoList[i], vNormal, 1);
		glVertexArrayAttribFormat(vaoList[i], vNormal, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3));

		if (meshList[i].texList.size() == 0) { // no texture coords mapped, so map everything to 0
			glVertexAttrib2f(vTexCoord, 0.0f, 0.0f);
		}
		else { // model has texcoords, send them too
			glCreateBuffers(1, &(vboList[i].uv));
			glNamedBufferStorage(vboList[i].uv, sizeof(glm::vec2) * meshList[i].texList.size(), meshList[i].texList.data(), 0);

			glVertexArrayVertexBuffer(vaoList[i], vTexCoord, vboList[i].uv, 0, sizeof(glm::vec2));
			//glBindBuffer(GL_ARRAY_BUFFER, vboList[i].uv);

			//glEnableVertexAttribArray(vTexCoord);
			//glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0); // should this be done only if a texture is loaded?

			glEnableVertexArrayAttrib(vaoList[i], vTexCoord);
			glVertexArrayAttribBinding(vaoList[i], vTexCoord, 3);
			glVertexArrayAttribFormat(vaoList[i], vTexCoord, 2, GL_FLOAT, GL_FALSE, 0);
		}

		glCreateBuffers(1, &(vboList[i].elem)); // model definitely has elements
		glNamedBufferStorage(vboList[i].elem, sizeof(unsigned short) * meshList[i].elemList.size(), meshList[i].elemList.data(), 0);
		
		glVertexArrayElementBuffer(vaoList[i], vboList[i].elem);

		meshInfo.push_back(meshlite{});

		meshInfo[i].color = meshList[i].color;
		meshInfo[i].psize = (int)(meshList[i].pList.size() * sizeof(pt));
		meshInfo[i].tsize = (int)(meshList[i].texList.size() * sizeof(glm::vec2));
		meshInfo[i].esize = (int)(meshList[i].elemList.size() * sizeof(unsigned short));
		int offset = 0;
		for (int j = 0; j < i; j++) {
			offset += (int)meshList[i].elemList.size(); // offsets should be ok here idk
		}
		meshInfo[i].eoffset = offset;
	}
	return true;
}

model& model::operator=(const model& other) { // only gpu state
	if (this != &other) {
		meshInfo = other.meshInfo;

		for (int i = 0; i < other.meshInfo.size(); i++) {
			vboList.push_back(bufferGroup{ 0, 0, 0, 0 });
			vaoList.push_back(0);

			glCreateVertexArrays(1, &(vaoList[i]));
			glBindVertexArray(vaoList[i]);

			glCreateBuffers(1, &(vboList[i].pos)); // model definitely has position vertices
			glNamedBufferStorage(vboList[i].pos, other.meshInfo[i].psize, nullptr, 0);
			
			glCopyNamedBufferSubData(other.vboList[i].pos, vboList[i].pos, 0, 0, other.meshInfo[i].psize);
			glBindBuffer(GL_ARRAY_BUFFER, vboList[i].pos);

			glEnableVertexAttribArray(vPosition);
			glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, sizeof(pt), (GLvoid*)0);

			glEnableVertexAttribArray(vNormal);
			glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, sizeof(pt), (GLvoid*)sizeof(glm::vec3)); // normals stored with vertices

			if (meshInfo[i].tsize == 0) { // no texture coords mapped, so map everything to 0
				glVertexAttrib2f(vTexCoord, 0.0f, 0.0f);
			}
			else { // model has texcoords, send them too
				glCreateBuffers(1, &(vboList[i].uv));
				glBindBuffer(GL_ARRAY_BUFFER, vboList[i].uv);
				glNamedBufferStorage(vboList[i].uv, other.meshInfo[i].tsize, nullptr, 0);

				glCopyNamedBufferSubData(other.vboList[i].uv, vboList[i].uv, 0, 0, other.meshInfo[i].tsize);

				glEnableVertexAttribArray(vTexCoord);
				glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0); // should this be done only if a texture is loaded?
			}

			glCreateBuffers(1, &(vboList[i].elem)); // model definitely has elements
			glNamedBufferStorage(vboList[i].elem, other.meshInfo[i].esize, nullptr, 0);

			glVertexArrayElementBuffer(vaoList[i], vboList[i].elem); // new!
			glCopyNamedBufferSubData(other.vboList[i].elem, vboList[i].elem, 0, 0, other.meshInfo[i].esize);
		}
	}
	return *this;
}

bool model::loadTexture(texInfo &t, GLint binding) { // type of texture, path to texture, and sampler to bind the texture to.
	if (!vboList.size()) {
		cerr << "WARNING: Loading texture even though model data hasn't been loaded yet." << endl; // check the first mesh, if it doesn't have texture coords it's likely the rest of the model doesn't either
	}

	limage::image temp;
	temp.load(t, binding);
	if (t.handle) {
		texList.push_back(t);
	}

	return t.handle != 0;
}

bool model::loadTextureArray(texInfo* t, int numTextures, GLint binding) {
	if (!vboList.size()) {
		cerr << "WARNING: Loading texture even though model data hasn't been loaded yet." << endl;
	}

	limage::image tempCube;
	tempCube.loadArray(t, numTextures, binding);
	if (t[0].handle) {
		for (int i = 0; i < numTextures; i++) { // load all textures into texture cache, some may be used in the future
			texList.push_back(t[i]);
		}
	}

	return t != 0;
}

void model::draw(const int color) const {
	for (int i = 0; i < meshInfo.size(); i++) {
		glm::vec4 currColor = meshInfo[i].color;
		if (color) {
			glVertexAttrib4f(vColor, currColor.r, currColor.g, currColor.b, currColor.a);
		}

		glBindVertexArray(vaoList[i]);
		glDrawElements(GL_TRIANGLES, (GLsizei)meshInfo[i].esize, GL_UNSIGNED_SHORT, (GLvoid*)0);
	}
}

void model::drawInstanced(const int color, const int count) const {
	for (int i = 0; i < meshInfo.size(); i++) {
		glm::vec4 currColor = meshInfo[i].color;
		if (color) {
			glVertexAttrib4f(vColor, currColor.r, currColor.g, currColor.b, currColor.a);
		}

		glBindVertexArray(vaoList[i]);
		glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)meshInfo[i].esize, GL_UNSIGNED_SHORT, (GLvoid*)0, count);
	}
}

model::~model() {
	//cout << "Model destructor called." << endl;

	glDeleteVertexArrays((GLsizei)vaoList.size(), vaoList.data());

	for (auto i = vboList.begin(); i != vboList.end(); i++) { // if a buffer isn't actually bound when deleted, glDeleteBuffers silently ignores it.
		glDeleteBuffers(1, &(i->pos));
		glDeleteBuffers(1, &(i->norm));
		glDeleteBuffers(1, &(i->uv));
		glDeleteBuffers(1, &(i->elem));
	}

	for (auto i = texList.begin(); i != texList.end(); i++) { // if a buffer isn't actually bound when deleted, glDeleteBuffers silently ignores it.
		glDeleteTextures(1, &(i->handle));
	}
}