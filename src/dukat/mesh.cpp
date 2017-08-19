#include "stdafx.h"
#include "mesh.h"
#include "buffers.h"
#include "shaderprogram.h"
#include "log.h"
#include "renderer.h"

namespace dukat
{
	Mesh::Mesh(GLenum mode, int max_vertices, int max_indices, const std::vector<VertexAttribute>& attributes, bool static_mesh)
		: mode(mode), max_vertices(max_vertices), max_indices(max_indices), attributes(attributes), static_mesh(static_mesh)
	{
		// Revisit this if we want to support meshes with more than 64k vertices
		assert((max_vertices - 1) <= std::numeric_limits<unsigned short>::max());
		logger << "Creating mesh: [vertices=" << max_vertices << ",indices=" << max_indices << ",static=" << static_mesh;
		// compute stride and offset
		GLsizei stride = 0;
		for (auto& it : this->attributes)
		{
			if (it.offset < 0)
			{
				it.offset = (int)stride;
			}
			if (it.type == GL_FLOAT)
			{
				stride += it.components * sizeof(GLfloat);
			}
			else if (it.type == GL_SHORT)
			{
				stride += it.components * sizeof(GLshort);
			}
		}
		logger << ",stride=" << stride << "]" << std::endl;

		buffer = std::make_unique<VertexBuffer>(max_indices > 0 ? 2 : 1);
		buffer->load_data(0, GL_ARRAY_BUFFER, max_vertices, stride, nullptr, 
			static_mesh ? GL_STATIC_DRAW : GL_STREAM_DRAW);
		if (max_indices > 0)
		{
			buffer->load_data(1, GL_ELEMENT_ARRAY_BUFFER, max_indices, sizeof(GLushort), 
				nullptr, GL_STATIC_DRAW);
		}
	}

	void Mesh::set_vertices(const std::vector<GLfloat>& vertices, int vertex_count)
	{
		set_vertices(vertices.data(), vertex_count);
	}

	void Mesh::set_vertices(const std::vector<GLshort>& vertices, int vertex_count)
	{
		set_vertices(vertices.data(), vertex_count);
	}

	void Mesh::set_vertices(const GLvoid* vertices, int vertex_count)
	{
		buffer->counts[0] = vertex_count > 0 ? vertex_count : max_vertices;
		glBindBuffer(GL_ARRAY_BUFFER, buffer->buffers[0]);
		if (static_mesh)
		{
			glBufferData(GL_ARRAY_BUFFER, buffer->counts[0] * buffer->strides[0], vertices, GL_STATIC_DRAW);
		}
		else
		{
			// Orphan buffer to improve streaming performance
			glBufferData(GL_ARRAY_BUFFER, max_vertices * buffer->strides[0], nullptr, GL_STREAM_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER, 0, buffer->counts[0] * buffer->strides[0], vertices);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void Mesh::set_indices(const std::vector<GLushort>& indices, int index_count)
	{
		set_indices(indices.data(), index_count);
	}

	void Mesh::set_indices(const GLvoid* indices, int index_count)
	{
		assert(buffer->buffer_count > 1);
		buffer->counts[1] = index_count > 0 ? index_count : max_indices;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->buffers[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer->counts[1] * buffer->strides[1], indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	
	void Mesh::render(ShaderProgram* program)
	{
#if OPENGL_VERSION >= 30
		glBindVertexArray(buffer->vao);
		glBindBuffer(GL_ARRAY_BUFFER, buffer->buffers[0]);

		for (auto& attr : attributes)
		{
			auto pid = program->attr(attr.alias);
			if (pid == -1)
				continue;
			glEnableVertexAttribArray(pid);
			glVertexAttribPointer(pid, attr.components, attr.type, GL_FALSE, buffer->strides[0],
				reinterpret_cast<const GLvoid*>(attr.offset));
		}
#else
		glBindBuffer(GL_ARRAY_BUFFER, buffer->buffers[0]);
		
		// TODO: check performance of the following:
		for (auto& attr : attributes)
		{
			if (attr.alias == Renderer::at_pos)
			{
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(attr.components, attr.type, buffer->strides[0],
					reinterpret_cast<const GLvoid*>(attr.offset));
			}
			else if (attr.alias == Renderer::at_color)
			{
				glEnableClientState(GL_COLOR_ARRAY);
				glColorPointer(attr.components, attr.type, buffer->strides[0],
					reinterpret_cast<const GLvoid*>(attr.offset));
			}
			else if (attr.alias == Renderer::at_texcoord)
			{
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(attr.components, attr.type, buffer->strides[0],
					reinterpret_cast<const GLvoid*>(attr.offset));
			}
			else if (attr.alias == Renderer::at_normal)
			{
				glEnableClientState(GL_NORMAL_ARRAY);
				glNormalPointer(attr.type, buffer->strides[0],
					reinterpret_cast<const GLvoid*>(attr.offset));
			}
		}
#endif

		if (max_indices > 0)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->buffers[1]);
			glDrawElements(mode, buffer->counts[1], GL_UNSIGNED_SHORT, static_cast<GLvoid*>(0));
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
		else
		{
			glDrawArrays(mode, 0, buffer->counts[0]);
		}

#if OPENGL_VERSION >= 30
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
#else
	#ifdef _DEBUG
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	#endif
#endif

		perfc.inc(PerformanceCounter::MESHES);
		perfc.inc(PerformanceCounter::VERTICES, buffer->counts[0]);
	}
}