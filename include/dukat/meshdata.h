#pragma once

#include <memory>
#include <string>
#include <vector>

#ifndef OPENGL_VERSION
#include "version.h"
#endif // !OPENGL_VERSION

#include "buffers.h"

namespace dukat
{
	// Forward declarations
	class ShaderProgram;

	struct VertexAttribute
	{
		const std::string alias;
		const int components;
		const GLenum type;
		int offset;

		VertexAttribute(const std::string& alias, int components, int offset = -1, GLenum type = GL_FLOAT)
			: alias(alias), components(components), type(type), offset(offset) { };
	};

	class MeshData
	{
	private:
		const GLenum mode;
		const int max_vertices;
		const int max_indices;
		const bool static_mesh;
		std::vector<VertexAttribute> attributes;
		// Buffer containing vertex data at index 0, and optionally index data at index 1.
		std::unique_ptr<VertexBuffer> buffer;

	public:
		// Creates a new mesh with given attributes.
		MeshData(GLenum mode, int max_vertices, int max_indices, 
			const std::vector<VertexAttribute>& attributes, bool static_mesh = true);
		~MeshData(void) { }

		// Sets vertices and indices. If not provided, count is assumed to be 
		// max count as provided during mesh construction.
		void set_vertices(const std::vector<GLfloat>& vertices, int vertex_count = -1);
		void set_vertices(const std::vector<GLshort>& vertices, int vertex_count = -1);
		void set_vertices(const GLvoid* vertices, int vertex_count = -1);
		void set_indices(const std::vector<GLushort>& indicies, int index_count = -1);
		void set_indices(const GLvoid* indices, int index_count = -1);
		int vertex_count(void) const { return buffer->counts[0]; }
		void set_vertex_count(int count) { buffer->counts[0] = count; }

		// Renders this mesh.
		void render(ShaderProgram* program);
	};
}