#include "stdafx.h"
#include "mesh.h"
#include "buffers.h"
#include "shaderprogram.h"
#include "log.h"

namespace dukat
{
	Mesh::Mesh(GLenum mode, unsigned int maxVertices, unsigned int maxIndices, const std::vector<VertexAttribute>& attributes, bool static_mesh)
		: mode(mode), max_vertices(maxVertices), max_indices(maxIndices), attributes(attributes), static_mesh(static_mesh), stride(0)
	{
		logger << "Creating mesh: [vertices=" << max_vertices << ",indices=" << max_indices << ",static=" << static_mesh;

		// compute stride and offset
		int offset = 0;
		for (auto& it : this->attributes)
		{
			int size = it.components * sizeof(GLfloat);
			stride += size;
			if (it.offset < 0)
			{
				it.offset = offset;
			}
			offset += size;
		}

		logger << ",offset=" << offset << "]" << std::endl;

		buffer = std::make_unique<VertexBuffer>(max_indices > 0 ? 2 : 1);

		glBindBuffer(GL_ARRAY_BUFFER, buffer->buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, max_vertices * stride, nullptr, static_mesh ? GL_STATIC_DRAW : GL_STREAM_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		if (max_indices > 0)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->buffers[1]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_indices * sizeof(GLushort), nullptr, GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}

	void Mesh::set_vertices(const std::vector<GLfloat>& vertices, unsigned int vertex_count)
	{
		set_vertices(vertices.data(), vertex_count);
	}

	void Mesh::set_vertices(const GLfloat* vertices, unsigned int vertex_count)
	{
		actual_vertices = vertex_count > 0 ? (int)vertex_count : max_vertices;
		glBindBuffer(GL_ARRAY_BUFFER, buffer->buffers[0]);
		if (static_mesh)
		{
			glBufferData(GL_ARRAY_BUFFER, actual_vertices * stride, vertices, GL_STATIC_DRAW);
		}
		else
		{
			// Orphan buffer to improve streaming performance
			glBufferData(GL_ARRAY_BUFFER, max_vertices * stride, nullptr, GL_STREAM_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER, 0, actual_vertices * stride, vertices);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void Mesh::set_indices(const std::vector<GLushort>& indices, unsigned int indexCount)
	{
		set_indices(indices.data(), indexCount);
	}

	void Mesh::set_indices(const GLushort* indices, unsigned int indexCount)
	{
		actual_indices = indexCount > 0 ? (int)indexCount : max_indices;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->buffers[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, actual_indices * sizeof(GLushort), indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	
	void Mesh::render(ShaderProgram* program)
	{
		glBindVertexArray(buffer->vao);
		glBindBuffer(GL_ARRAY_BUFFER, buffer->buffers[0]);

		for (auto& attr : attributes)
		{
			auto pid = program->attr(attr.alias);
			if (pid == -1)
				continue;
			glEnableVertexAttribArray(pid);
			glVertexAttribPointer(pid, attr.components, GL_FLOAT, GL_FALSE, stride,
				reinterpret_cast<const GLvoid*>(attr.offset));
		}

		if (max_indices > 0)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->buffers[1]);
			glDrawElements(mode, actual_indices, GL_UNSIGNED_SHORT, static_cast<GLvoid*>(0));
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
		else
		{
			glDrawArrays(mode, 0, actual_vertices);
		}

		for (auto& attr : attributes)
		{
			auto pid = program->attr(attr.alias);
			if (pid == -1)
				continue;
			glDisableVertexAttribArray(pid);
		}

#ifdef _DEBUG
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
#endif
	}
}