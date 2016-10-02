#pragma once

#include <memory>

#include "material.h"
#include "mesh.h"
#include "texturecache.h"
#include "transform3.h"

namespace dukat
{
	struct GenericBuffer;
	class ShaderProgram;
	class Renderer;

	class MeshInstance
	{
	private:
		Mesh* mesh;
		std::unique_ptr<GenericBuffer> uniform_buffers;
		ShaderProgram* program;
		Texture* texture;
		Material material;

	public:
		ExtendedTransform3 transform;
		bool visible;

		MeshInstance(void);
		virtual ~MeshInstance(void) { }

		void set_mesh(Mesh* mesh) { this->mesh = mesh; }
		Mesh* get_mesh(void) const { return mesh; }
		void set_material(const Material& material);
		Material get_material(void) { return material; }
		void set_program(ShaderProgram* program) { this->program = program; }
		void set_texture(Texture* texture) { this->texture = texture; }
		// Renders mesh instance using transformation specified in mat.
		void render(Renderer* renderer, const Matrix4& mat);
	};
}
