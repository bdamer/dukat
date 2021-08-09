#include "stdafx.h"
#include <dukat/meshinstance.h>
#include <dukat/buffers.h>
#include <dukat/shaderprogram.h>
#include <dukat/perfcounter.h>
#include <dukat/renderer.h>

namespace dukat
{
	MeshInstance::MeshInstance(void) : Mesh(), program(nullptr)
	{
		for (auto i = 0; i < Renderer::max_texture_units; i++)
			texture[i] = nullptr;
#if OPENGL_VERSION >= 30
		uniform_buffers = std::make_unique<GenericBuffer>(1);
		glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(Renderer::UniformBuffer::Material), uniform_buffers->buffers[0]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), &material, GL_STATIC_DRAW);
#endif
		// initialize default transform
		transform.update();
	}

	void MeshInstance::update_material_buffer(void)
	{
#if OPENGL_VERSION >= 30
		glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(Renderer::UniformBuffer::Material), uniform_buffers->buffers[0]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), &material, GL_STATIC_DRAW);
#endif
	}

	void MeshInstance::set_texture(Texture* texture, int index)
	{
		assert(index >= 0 && index < Renderer::max_texture_units);
		this->texture[index] = texture;
	}

	void MeshInstance::render(Renderer* renderer)
	{
		Matrix4 mat;
		mat.identity();
		render(renderer, mat);
	}

	void MeshInstance::render(Renderer* renderer, const Matrix4& mat)
	{
		renderer->switch_shader(program);

		// combine entity and mesh model matrices
		Matrix4 model = mat * transform.mat_model;
		glUniformMatrix4fv(program->attr(Renderer::uf_model), 1, false, model.m);

#if OPENGL_VERSION >= 30
		// Bind uniform buffers
		if (program->bind(Renderer::uf_material, static_cast<GLuint>(Renderer::UniformBuffer::Material)))
			glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(Renderer::UniformBuffer::Material), uniform_buffers->buffers[0]);
#else
		// manually bind material (sending custom.r twice since there is no dedicated
		// shininess in later versions)
		glMaterialfv(GL_FRONT, GL_AMBIENT, (GLfloat*)(&material.ambient));
		glMaterialfv(GL_FRONT, GL_DIFFUSE, (GLfloat*)(&material.diffuse));
		glMaterialfv(GL_FRONT, GL_SPECULAR, (GLfloat*)(&material.specular));
		glMaterialfv(GL_FRONT, GL_EMISSION, (GLfloat*)(&material.custom));
		glMaterialf(GL_FRONT, GL_SHININESS, material.custom.r);
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
