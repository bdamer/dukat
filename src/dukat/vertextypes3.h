#pragma once

#include <GL/glew.h>
#include "color.h"

namespace dukat
{
	struct Vertex3P
	{
		GLfloat px, py, pz;
	};

	struct Vertex3PC
	{
		GLfloat px, py, pz;
		GLfloat cr, cb, cg, ca;
	};

	struct Vertex3PNT
	{
		GLfloat px, py, pz;
		GLfloat nx, ny, nz;
		GLfloat tu, tv;
	};

	struct Vertex3PT
	{
		GLfloat px, py, pz;
		GLfloat tu, tv;
	};

	struct Vertex3PCT
	{
		GLfloat px, py, pz;
		GLfloat cr, cb, cg, ca;
		GLfloat tu, tv;
	};
}
