#include "object.h"

obj::obj() : nelems(0) {
	glCreateVertexArrays(1, &vao);
	glCreateBuffers(1, &vbo);
	glCreateBuffers(1, &ebo);
}

obj::~obj() {
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
}

obj::obj(obj&& other) : vao(other.vao), vbo(other.vbo), ebo(other.ebo), nelems(other.nelems) {
	other.vao = 0;
	other.vbo = 0;
	other.ebo = 0;
	other.nelems = 0;
}

obj& obj::operator=(obj&& other) {
	if (this != &other) {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ebo);

		vao = other.vao;
		vbo = other.vbo;
		ebo = other.ebo;
		nelems = other.nelems;

		other.vao = 0;
		other.vbo = 0;
		other.ebo = 0;
		other.nelems = 0;
	}

	return *this;
}

// working with just one mesh for now...
void obj::load(vload::vloader vl) {
	vload::mesh m = vl.meshList[0];
	nelems = m.elemList.size();

	glNamedBufferStorage(vbo, m.pList.size() * sizeof(vload::pt), m.pList.data(), 0);
	
	glVertexArrayVertexBuffer(vao, vPosition, vbo, 0, sizeof(vload::pt));
	glEnableVertexArrayAttrib(vao, vPosition);
	glVertexArrayAttribBinding(vao, vPosition, 0);
	glVertexArrayAttribFormat(vao, vPosition, 3, GL_FLOAT, GL_FALSE, 0);

	glVertexArrayVertexBuffer(vao, vNormal, vbo, 0, sizeof(vload::pt));
	glEnableVertexArrayAttrib(vao, vNormal);
	glVertexArrayAttribBinding(vao, vNormal, 1);
	glVertexArrayAttribFormat(vao, vNormal, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));

	glNamedBufferStorage(ebo, nelems * sizeof(unsigned int), m.elemList.data(), 0);
	glVertexArrayElementBuffer(vao, ebo);
}

void obj::render() const {
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, nelems, GL_UNSIGNED_INT, nullptr);
}

