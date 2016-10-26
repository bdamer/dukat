#pragma once

#include <gl/GL.h>

namespace dukat
{
	struct Vertex2
	{
		GLfloat x, y;
	};

	struct TexturedVertex2
	{
		GLfloat x, y;
		GLfloat u, v;
	};

	struct ColoredVertex2
	{
		GLfloat x, y;
		GLfloat r, g, b, a;
	};

	struct ColoredTexturedVertex2
	{
		GLfloat x, y;
		GLfloat r, g, b, a;
		GLfloat u, v;
	};

	struct ParticleVertex2
	{
		GLfloat x, y;
		GLfloat size;
		GLfloat r, g, b, a;
	};
}