#include "stdafx.h"
#include "meshinstance.h"
#include "buffers.h"
#include "shaderprogram.h"
#include "perfcounter.h"
#include "renderer.h"

namespace dukat
{
	MeshInstance::MeshInstance(void) : program(nullptr), visible(true)
	{
		for (auto i = 0; i < Renderer::max_texture_units; i++)
			texture[i] = nullptr;
		uniform_buffers = std::make_unique<GenericBuffer>(1);
		glBindBufferBase(GL_UNIFORM_BUFFER, Renderer::UniformBuffer::MATERIAL, uniform_buffers->buffers[0]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), &material, GL_STATIC_DRAW);
		// initialize default transform
		transform.update();
	}

	void MeshInstance::set_material(const Material& material)
	{
		this->material = material;
		glBindBufferBase(GL_UNIFORM_BUFFER, Renderer::UniformBuffer::MATERIAL, uniform_buffers->buffers[0]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), &material, GL_STATIC_DRAW);
	}

	void MeshInstance::set_texture(Texture* texture, int index)
	{
		assert(index >= 0 && index < Renderer::max_texture_units);
		this->texture[index] = texture;
	}

	void MeshInstance::render(Renderer* renderer, const Matrix4& mat)
	{
		renderer->switch_shader(program);

		// combine entity and mesh model matrices
		Matrix4 model = mat * transform.mat_model;
		glUniformMatrix4fv(program->attr(Renderer::uf_model), 1, false, model.m);

		// Bind uniform buffers
		// TODO: use a single buffer for all materials?
		glUniformBlockBinding(program->id, glGetUniformBlockIndex(program->id, Renderer::uf_material), Renderer::UniformBuffer::MATERIAL);
		glBindBufferBase(GL_UNIFORM_BUFFER, Renderer::UniformBuffer::MATERIAL, uniform_buffers->buffers[0]);

		// Bind texture
		std::string id = "u_tex0"; 
		for (auto i = 0; i < Renderer::max_texture_units; i++)
		{
			if (texture[i] != nullptr)
			{
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, texture[i]->id);
				id[5] = static_cast<char>(48 + i);
				glUniform1i(program->attr(id), i);
			}
		}

		mesh->render(program);

		perfc.inc(PerformanceCounter::MESHES);
		perfc.inc(PerformanceCounter::VERTICES, mesh->vertex_count());
	}
}
