#pragma once

#include <memory>

#include "material.h"
#include "mesh.h"
#include "meshdata.h"
#include "texturecache.h"
#include "transform3.h"
#include "renderer.h"

namespace dukat
{
	struct GenericBuffer;
	class ShaderProgram;

	class MeshInstance : public Mesh
	{
	private:
		std::string name;
		MeshData* mesh;
#if OPENGL_VERSION >= 30
		std::unique_ptr<GenericBuffer> uniform_buffers;
#endif
		ShaderProgram* program;
		Texture* texture[Renderer::max_texture_units];
		Material material;

	public:
		MeshInstance(void);
		virtual ~MeshInstance(void) { }

		void set_name(const std::string& name) { this->name = name; }
		const std::string& get_name(void) const { return name; }
		void set_mesh(MeshData* mesh) { this->mesh = mesh; }
		MeshData* get_mesh(void) const { return mesh; }
		void set_material(const Material& material);
		Material get_material(void) const { return material; }
		void set_program(ShaderProgram* program) { this->program = program; }
		void set_texture(Texture* texture, int index = 0);
		// Updates mesh transform.		
		virtual void update(float delta) { this->transform.update(); }
		// Renders mesh instance using only local transformation.
		virtual void render(Renderer* renderer);
		// Renders mesh instance using transformation specified in mat.
		virtual void render(Renderer* renderer, const Matrix4& mat);
	};
}
