#pragma once

namespace dukat
{
	struct Vertex2P
	{
		GLfloat px, py;
	};

	struct Vertex2PT
	{
		GLfloat px, py;
		GLfloat tu, tv;
	};

	struct Vertex2PC
	{
		GLfloat px, py;
		GLfloat cr, cg, cb, ca;
	};

	struct Vertex2PCT
	{
		GLfloat px, py;
		GLfloat cr, cg, cb, ca;
		GLfloat tu, tv;
	};

	struct Vertex2PSC
	{
		GLfloat px, py;
		GLfloat size;
		GLfloat cr, cg, cb, ca;
	};

	struct Vertex2PSRC
	{
		GLfloat px, py;
		GLfloat size;
		GLfloat ry;
		GLfloat cr, cg, cb, ca;
	};
}