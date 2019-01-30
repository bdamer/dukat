#pragma once

#include <memory>

#ifndef OPENGL_VERSION
#include "version.h"
#endif // !OPENGL_VERSION

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

		void update_material_buffer(void);

	public:
		MeshInstance(void);
		virtual ~MeshInstance(void) { }

		void set_name(const std::string& name) { this->name = name; }
		const std::string& get_name(void) const { return name; }
		void set_mesh(MeshData* mesh) { this->mesh = mesh; }
		MeshData* get_mesh(void) const { return mesh; }
		void set_material(const Material& material) { this->material = material; update_material_buffer(); }
		Material get_material(void) const { return material; }
		void set_ambient(const Color& ambient) { this->material.ambient = ambient; update_material_buffer(); }
		Color get_ambient(void) const { return material.ambient; }
		void set_diffuse(const Color& diffuse) { this->material.diffuse = diffuse; update_material_buffer(); }
		Color get_diffuse(void) const { return material.diffuse; }
		void set_specular(const Color& specular) { this->material.specular = specular; update_material_buffer(); }
		Color get_specular(void) const { return material.specular; }
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
