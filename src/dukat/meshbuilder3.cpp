#include "stdafx.h"
#include "meshbuilder3.h"
#include "mesh.h"
#include "vertextypes3.h"
#include "geometry.h"
#include "buffers.h"
#include "renderer.h"
#include "mathutil.h"
#include "vector3.h"

namespace dukat
{
	std::unique_ptr<Mesh> MeshBuilder3::build_axis(void)
	{
		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3, offsetof(VertexPosCol, pos)));
		attr.push_back(VertexAttribute(Renderer::at_color, 4, offsetof(VertexPosCol, col)));

		VertexPosCol vertices[6] = {
			0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f
		};

		auto res = std::make_unique<Mesh>(GL_LINES, 6, 0, attr);
		res->set_vertices(reinterpret_cast<GLfloat*>(vertices));
		return res;
	}

	std::unique_ptr<Mesh> MeshBuilder3::build_points(const std::vector<Vector3>& points, const Color& color)
	{
		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3, offsetof(VertexPosCol, pos)));
		attr.push_back(VertexAttribute(Renderer::at_color, 4, offsetof(VertexPosCol, col)));

		std::vector<VertexPosCol> vertices;
		for (const auto& it : points)
		{
			vertices.push_back({ it.x, it.y, it.z, color.r, color.g, color.b, color.a });
		}

		auto res = std::make_unique<Mesh>(GL_POINTS, (unsigned int)vertices.size(), 0, attr);
		res->set_vertices(reinterpret_cast<GLfloat*>(vertices.data()));
		return res;
	}

	std::unique_ptr<Mesh> MeshBuilder3::build_cube(void)
	{
		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3, offsetof(VertexPosNorTex, pos)));
		attr.push_back(VertexAttribute(Renderer::at_normal, 3, offsetof(VertexPosNorTex, nor)));
		attr.push_back(VertexAttribute(Renderer::at_texcoord, 2, offsetof(VertexPosNorTex, u)));

		VertexPosNorTex verts[36] = {
			// Down
			-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.25f, 0.25f,
			 1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.50f, 0.25f,
			-1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f, 0.25f, 0.50f,
			 1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.50f, 0.25f,
			 1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f, 0.50f, 0.50f,
			-1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f, 0.25f, 0.50f,
			// Top
			-1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.25f, 0.75f,
			 1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.50f, 0.75f,
			-1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.25f, 0.0f,
			 1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.50f, 0.75f,
			 1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.50f, 0.0f,
			-1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.25f, 0.0f,
			// Back
			 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.75f, 0.50f,
			-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.00f, 0.50f,
			 1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.75f, 0.75f,
			-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.00f, 0.50f,
			-1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.00f, 0.75f,
			 1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.75f, 0.75f,
			// Front
			-1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.25f, 0.50f,
			 1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.50f, 0.50f,
			-1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.25f, 0.75f,
			 1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.50f, 0.50f,
			 1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.50f, 0.75f,
			-1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.25f, 0.75f,
			// Left
			-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.00f, 0.50f,
			-1.0f, -1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 0.25f, 0.50f,
			-1.0f,  1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.00f, 0.75f,
			-1.0f, -1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 0.25f, 0.50f,
			-1.0f,  1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 0.25f, 0.75f,
			-1.0f,  1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.00f, 0.75f,
			// Right
			 1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.50f, 0.50f,
			 1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.75f, 0.50f,
			 1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.50f, 0.75f,
			 1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.75f, 0.50f,
			 1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.75f, 0.75f,
			 1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.50f, 0.75f
		};

		auto res = std::make_unique<Mesh>(GL_TRIANGLES, 36, 0, attr);
		res->set_vertices(reinterpret_cast<GLfloat*>(verts));
		return res;
	}

	std::unique_ptr<Mesh> MeshBuilder3::build_sphere(int slices, int stacks, bool invert)
	{
		auto vertices = generate_sphere(1.0f, slices, stacks, invert);
		
		int numVertices = static_cast<int>(vertices.size()) / 6;
		int numIndices = (slices + 1) * 2 * stacks;

		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3));
		attr.push_back(VertexAttribute(Renderer::at_normal, 3));

		auto res = std::make_unique<Mesh>(GL_TRIANGLE_STRIP, numVertices, numIndices, attr);
		res->set_vertices(vertices, numVertices);

		// First, generate vertex index arrays for drawing with glDrawElements
		// All stacks, including top and bottom are covered with a triangle
		// strip.
		std::vector<GLushort> indices(numIndices);

		// top stack
		int idx = 0;
		for (int j = 0; j < slices; j++) {
			indices[idx++] = (GLushort)(j + 1); // 0 is top vertex, 1 is first for first stack
			indices[idx++] = 0;
		}
		indices[idx++] = (GLushort)1; // repeat first slice's idx for closing off shape
		indices[idx++] = 0;

		// middle stacks:
		// Strip indices are relative to first index belonging to strip, NOT relative to first vertex/normal pair in array
		int offset;
		for (int i = 0; i < stacks - 2; i++) {
			offset = 1 + i * slices;                    // triangle_strip indices start at 1 (0 is top vertex), and we advance one stack down as we go along
			for (int j = 0; j < slices; j++) {
				indices[idx++] = (GLushort)(offset + j + slices);
				indices[idx++] = (GLushort)(offset + j);
			}
			indices[idx++] = (GLushort)(offset + slices);      // repeat first slice's idx for closing off shape
			indices[idx++] = (GLushort)offset;
		}

		// bottom stack
		offset = 1 + (stacks - 2) * slices;  // triangle_strip indices start at 1 (0 is top vertex), and we advance one stack down as we go along
		for (int j = 0; j < slices; j++)
		{
			indices[idx++] = (GLushort)(numVertices - 1); // zero based index, last element in array (bottom vertex)...
			indices[idx++] = (GLushort)(offset + j);
		}
		indices[idx++] = (GLushort)(numVertices - 1);   // repeat first slice's idx for closing off shape
		indices[idx++] = (GLushort)offset;

		res->set_indices(indices);

		return res;
	}
}
