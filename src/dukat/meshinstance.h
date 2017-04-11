#pragma once

#include <memory>

#include "material.h"
#include "mesh.h"
#include "texturecache.h"
#include "transform3.h"
#include "renderer.h"

namespace dukat
{
	struct GenericBuffer;
	class ShaderProgram;

	class MeshInstance
	{
	private:
		Mesh* mesh;
#if OPENGL_VERSION >= 30
		std::unique_ptr<GenericBuffer> uniform_buffers;
#endif
		ShaderProgram* program;
		Texture* texture[Renderer::max_texture_units];
		Material material;

	public:
		ExtendedTransform3 transform;
		bool visible;

		MeshInstance(void);
		virtual ~MeshInstance(void) { }

		void set_mesh(Mesh* mesh) { this->mesh = mesh; }
		Mesh* get_mesh(void) const { return mesh; }
		void set_material(const Material& material);
		Material get_material(void) const { return material; }
		void set_program(ShaderProgram* program) { this->program = program; }
		void set_texture(Texture* texture, int index = 0);
		// Renders mesh instance using transformation specified in mat.
		void render(Renderer* renderer, const Matrix4& mat);
	};
}
