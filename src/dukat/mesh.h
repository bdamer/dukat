#pragma once

#include <memory>
#include <string>
#include <vector>
#include <gl/glew.h>
#include "buffers.h"

namespace dukat
{
	// Forward declarations
	class ShaderProgram;

	struct VertexAttribute
	{
		const std::string alias;
		const int components;
		int offset;
		VertexAttribute(const std::string& alias, int components, int offset = -1)
			: alias(alias), components(components), offset(offset) { };
	};

	class Mesh
	{
	private:
		const GLenum mode;
		const unsigned int max_vertices;
		const unsigned int max_indices;
		const bool static_mesh;
		std::vector<VertexAttribute> attributes;
		unsigned int actual_vertices;
		unsigned int actual_indices;
		std::unique_ptr<VertexBuffer> buffer;
		GLsizei stride;

	public:
		// Creates a new mesh with given attributes.
		Mesh(GLenum mode, unsigned int max_vertices, unsigned int max_indices, 
			const std::vector<VertexAttribute>& attributes, bool static_mesh = true);
		~Mesh(void) { }

		// Sets vertices and indices. If not provided, count is assumed to be 
		// max count as provided during mesh construction.
		void set_vertices(const std::vector<GLfloat>& vertices, unsigned int vertex_count = 0);
		void set_vertices(const GLfloat* vertices, unsigned int vertex_count = 0);
		void set_indices(const std::vector<GLushort>& indicies, unsigned int index_count = 0);
		void set_indices(const GLushort* indices, unsigned int index_count = 0);
		int vertex_count(void) const { return actual_vertices; }

		// Renders this mesh.
		void render(ShaderProgram* program);
	};
}