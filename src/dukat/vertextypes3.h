#pragma once

#include <gl/glew.h>
#include "color.h"

namespace dukat
{
	struct Vector
	{
		GLfloat x, y, z;
	};

	struct VertexPos
	{
		Vector pos;
	};

	struct VertexPosNorTex
	{
		Vector pos;
		Vector nor;
		GLfloat u, v;
	};

	struct VertexPosTex
	{
		Vector pos;
		GLfloat u, v;
	};

	struct VertexPosCol
	{
		Vector pos;
		Color col;
	};

	struct VertexPosColTex
	{
		Vector pos;
		Color col;
		GLfloat u, v;
	};
}
