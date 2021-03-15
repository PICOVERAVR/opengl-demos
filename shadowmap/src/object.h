#pragma once
#include "window.h"
#include "vloader.h"
#include <iostream>

// the classic object class, encapsulating OpenGL state and default shaders to use when rendering.
// assuming an OpenGL context exists when we call the constructor.

enum {
	vPosition = 0,
	vNormal = 1,
};

class obj {
public:
	obj();
	~obj();
	
	// copy constructors are super slow here and it would't make a lot of sense to use them.
	obj(const obj& other)=delete;
	obj& operator=(const obj& other)=delete;
	
	obj(obj&& other);
	obj& operator=(obj&& other);

	void load(vload::vloader vl);
	void render() const;

	GLuint vao; // helpful to enable or disable vertex array parameters at runtime
private:
	GLuint vbo;
	GLuint ebo;
	unsigned int nelems;
};
