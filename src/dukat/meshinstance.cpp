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
#if OPENGL_VERSION >= 30
		uniform_buffers = std::make_unique<GenericBuffer>(1);
		glBindBufferBase(GL_UNIFORM_BUFFER, Renderer::UniformBuffer::MATERIAL, uniform_buffers->buffers[0]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), &material, GL_STATIC_DRAW);
#endif
		// initialize default transform
		transform.update();
	}

	void MeshInstance::set_material(const Material& material)
	{
		this->material = material;
#if OPENGL_VERSION >= 30
		glBindBufferBase(GL_UNIFORM_BUFFER, Renderer::UniformBuffer::MATERIAL, uniform_buffers->buffers[0]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), &material, GL_STATIC_DRAW);
#endif
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

#if OPENGL_VERSION >= 30
		// Bind uniform buffers
		// TODO: use a single buffer for all materials?
		glUniformBlockBinding(program->id, glGetUniformBlockIndex(program->id, Renderer::uf_material), Renderer::UniformBuffer::MATERIAL);
		glBindBufferBase(GL_UNIFORM_BUFFER, Renderer::UniformBuffer::MATERIAL, uniform_buffers->buffers[0]);
#else
		// manually bind material buffer
		glUniform4fv(program->attr("u_material_ambient"), 1, (GLfloat*)(&material.ambient));
		glUniform4fv(program->attr("u_material_diffuse"), 1, (GLfloat*)(&material.diffuse));
		glUniform4fv(program->attr("u_material_specular"), 1, (GLfloat*)(&material.specular));
		glUniform4fv(program->attr("u_material_custom"), 1, (GLfloat*)(&material.custom));
#endif

		// Bind texture
		for (auto i = 0; i < Renderer::max_texture_units; i++)
		{
			if (texture[i] != nullptr)
			{
				texture[i]->bind(i, program);
			}
		}

		mesh->render(program);
	}
}
