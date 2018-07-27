#include "stdafx.h"

#include "model3.h"

#include "gamebase.h"
#include "log.h"
#include "meshcache.h"
#include "meshgroup.h"
#include "sysutil.h"
#include "vertextypes3.h"

namespace dukat
{
    constexpr size_t Model3::string_length;
    constexpr uint32_t Model3::model_id;
    constexpr uint32_t Model3::model_version;

    Model3::Model3()
	{
		header.id = model_id;
		header.version = model_version;
		update_headers();
	}

	Model3::~Model3(void)
	{
	}

	void Model3::update_headers(void)
	{
		uint32_t offset = static_cast<uint32_t>(sizeof(Model3::Header));
		header.mesh_offset = offset;
		header.mesh_count = static_cast<uint32_t>(meshes.size());
		
		uint32_t index_offset = 0, vertex_offset = 0;
		for (uint32_t i = 0; i < header.mesh_count; i++)
		{
			meshes[i].index_offset = index_offset;
			meshes[i].vertex_offset = vertex_offset;			
			index_offset += meshes[i].index_count;
			vertex_offset += meshes[i].vertex_count;
		}
		offset += header.mesh_count * static_cast<uint32_t>(sizeof(Model3::Mesh));

		header.index_offset = offset;
		header.index_count = static_cast<uint32_t>(indices.size());
		offset += static_cast<uint32_t>(indices.size() * sizeof(GLushort));

		header.vertex_offset = offset;
		header.vertex_count = static_cast<uint32_t>(vertices.size());
	}

	void Model3::set_name(const std::string& name)
	{
		memcpy(header.name, name.c_str(), std::min(name.length(), string_length));
	}

	void Model3::add_mesh(const std::string& name, const Material& material, const std::string& texture,
		const Transform3& transform, const std::vector<GLushort>& mesh_indices, const std::vector<Model3::Vertex>& mesh_vertices)
	{
		log->trace("Adding mesh: {}", name);
		// copy mesh data
		indices.insert(indices.end(), mesh_indices.begin(), mesh_indices.end());
        vertices.insert(vertices.end(), mesh_vertices.begin(), mesh_vertices.end());

		// create mesh entry
		meshes.push_back(Model3::Mesh());
		auto& mesh = meshes.back();
		mesh.id = compute_hash(name);
		memcpy(mesh.name, name.c_str(), std::min(name.length(), string_length));
		memcpy(mesh.texture, texture.c_str(), std::min(texture.length(), string_length));
		mesh.material = material;
		mesh.transform = transform;
		mesh.index_count = static_cast<uint32_t>(mesh_indices.size());
		mesh.vertex_count = static_cast<uint32_t>(mesh_vertices.size());
		update_headers();
	}

	std::ostream& operator<<(std::ostream& os, const Model3& v)
	{
		os.write(reinterpret_cast<const char*>(&v.header.id), sizeof(uint32_t));
		os.write(reinterpret_cast<const char*>(&v.header.version), sizeof(uint32_t));
		os.write(reinterpret_cast<const char*>(&v.header.mesh_offset), sizeof(uint32_t));
		os.write(reinterpret_cast<const char*>(&v.header.mesh_count), sizeof(uint32_t));
		os.write(reinterpret_cast<const char*>(&v.header.index_offset), sizeof(uint32_t));
		os.write(reinterpret_cast<const char*>(&v.header.index_count), sizeof(uint32_t));
		os.write(reinterpret_cast<const char*>(&v.header.vertex_offset), sizeof(uint32_t));
		os.write(reinterpret_cast<const char*>(&v.header.vertex_count), sizeof(uint32_t));

		for (auto i = 0u; i < v.header.mesh_count; i++)
		{
			auto& m = v.meshes[i];
			os.write(reinterpret_cast<const char*>(&m.id), sizeof(uint32_t));
			os.write(reinterpret_cast<const char*>(&m.name), sizeof(uint8_t) * Model3::string_length);
			os.write(reinterpret_cast<const char*>(&m.texture), sizeof(uint8_t) * Model3::string_length);
			os.write(reinterpret_cast<const char*>(&m.material.ambient), sizeof(GLfloat) * 4);
			os.write(reinterpret_cast<const char*>(&m.material.diffuse), sizeof(GLfloat) * 4);
			os.write(reinterpret_cast<const char*>(&m.material.specular), sizeof(GLfloat) * 4);
			os.write(reinterpret_cast<const char*>(&m.material.custom), sizeof(GLfloat) * 4);
			os.write(reinterpret_cast<const char*>(&m.transform.position), sizeof(GLfloat) * 4);
			os.write(reinterpret_cast<const char*>(&m.transform.dir), sizeof(GLfloat) * 4);
			os.write(reinterpret_cast<const char*>(&m.transform.up), sizeof(GLfloat) * 4);
			os.write(reinterpret_cast<const char*>(&m.transform.left), sizeof(GLfloat) * 4);
			os.write(reinterpret_cast<const char*>(&m.transform.scale), sizeof(GLfloat) * 4);
			os.write(reinterpret_cast<const char*>(&m.index_offset), sizeof(uint32_t));
			os.write(reinterpret_cast<const char*>(&m.index_count), sizeof(uint32_t));
			os.write(reinterpret_cast<const char*>(&m.vertex_offset), sizeof(uint32_t));
			os.write(reinterpret_cast<const char*>(&m.vertex_count), sizeof(uint32_t));
		}

		os.write(reinterpret_cast<const char*>(v.indices.data()), sizeof(GLushort) * v.indices.size());
		os.write(reinterpret_cast<const char*>(v.vertices.data()), sizeof(Vertex3PNT) * v.vertices.size());

		return os;
	}

	std::istream& operator>>(std::istream& is, Model3& v)
	{
		log->info("Loading model.");

		is.read(reinterpret_cast<char*>(&v.header.id), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&v.header.version), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&v.header.mesh_offset), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&v.header.mesh_count), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&v.header.index_offset), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&v.header.index_count), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&v.header.vertex_offset), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&v.header.vertex_count), sizeof(uint32_t));
		if (v.header.id != Model3::model_id || v.header.version != Model3::model_version)
		{
			throw std::runtime_error("Invalid model format or version!");
		}

		v.meshes.resize(v.header.mesh_count);
		for (auto i = 0u; i < v.header.mesh_count; i++)
		{
			auto& m = v.meshes[i];
			is.read(reinterpret_cast<char*>(&m.id), sizeof(uint32_t));
			is.read(reinterpret_cast<char*>(&m.name), sizeof(uint8_t) * Model3::string_length);
			is.read(reinterpret_cast<char*>(&m.texture), sizeof(uint8_t) * Model3::string_length);
			is.read(reinterpret_cast<char*>(&m.material.ambient), sizeof(GLfloat) * 4);
			is.read(reinterpret_cast<char*>(&m.material.diffuse), sizeof(GLfloat) * 4);
			is.read(reinterpret_cast<char*>(&m.material.specular), sizeof(GLfloat) * 4);
			is.read(reinterpret_cast<char*>(&m.material.custom), sizeof(GLfloat) * 4);
			is.read(reinterpret_cast<char*>(&m.transform.position), sizeof(GLfloat) * 4);
			is.read(reinterpret_cast<char*>(&m.transform.dir), sizeof(GLfloat) * 4);
			is.read(reinterpret_cast<char*>(&m.transform.up), sizeof(GLfloat) * 4);
			is.read(reinterpret_cast<char*>(&m.transform.left), sizeof(GLfloat) * 4);
			is.read(reinterpret_cast<char*>(&m.transform.scale), sizeof(GLfloat) * 4);
			is.read(reinterpret_cast<char*>(&m.index_offset), sizeof(uint32_t));
			is.read(reinterpret_cast<char*>(&m.index_count), sizeof(uint32_t));
			is.read(reinterpret_cast<char*>(&m.vertex_offset), sizeof(uint32_t));
			is.read(reinterpret_cast<char*>(&m.vertex_count), sizeof(uint32_t));

			log->debug("Found mesh: {}", m.name);
		}

		v.indices.resize(v.header.index_count);
		is.read(reinterpret_cast<char*>(v.indices.data()), sizeof(GLushort) * v.indices.size());
		v.vertices.resize(v.header.vertex_count);
		is.read(reinterpret_cast<char*>(v.vertices.data()), sizeof(Vertex3PNT) * v.vertices.size());

		return is;
	}

	AABB3 Model3::create_aabb(void) const
	{
		AABB3 res;
		for (auto& mesh : meshes)
		{
			ExtendedTransform3 t(mesh.transform);
			t.update();
			for (auto i = mesh.vertex_offset; i < mesh.vertex_offset + mesh.vertex_count; i++)
			{
				Vector3 v = { vertices[i].pos[0], vertices[i].pos[1], vertices[i].pos[2] };
				v *= t.mat_model;
				res.add(v);
			}
		}
		return res;
	}

	std::unique_ptr<MeshGroup> build_mesh_group(GameBase* game, const Model3& model)
	{
		auto res = std::make_unique<MeshGroup>();

		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(dukat::Renderer::at_pos, 3, offsetof(Vertex3PNT, px)));
		attr.push_back(VertexAttribute(dukat::Renderer::at_normal, 3, offsetof(Vertex3PNT, nx)));
		attr.push_back(VertexAttribute(dukat::Renderer::at_texcoord, 2, offsetof(Vertex3PNT, tu)));

		auto mesh_cache = game->get_meshes();

		// Create instance for each mesh
		const auto& indices = model.get_indices();
		const auto& vertices = model.get_vertices();
		for (const auto& m : model.get_meshes())
		{
			MeshData* mesh;
			std::string mesh_id = model.get_name() + "|" + m.name;
			if (mesh_cache->contains(mesh_id))
			{
				mesh = mesh_cache->get(mesh_id);
			}
			else
			{
				auto src_mesh = std::make_unique<MeshData>(GL_TRIANGLES, m.vertex_count, m.index_count, attr);
				src_mesh->set_vertices(reinterpret_cast<const GLfloat*>(vertices.data() + m.vertex_offset), m.vertex_count);
				if (m.index_count > 0)
				{
					src_mesh->set_indices(reinterpret_cast<const GLushort*>(indices.data() + m.index_offset), m.index_count);
				}
				mesh = src_mesh.get();
				// Store mesh in cache
				mesh_cache->put(mesh_id, std::move(src_mesh));
			}

			// Create mesh instance
			auto instance = res->create_instance();
			instance->set_name(m.name);
			instance->set_mesh(mesh);
			std::string texture(m.texture);
			if (texture.length() > 0)
			{
				instance->set_texture(game->get_textures()->get(texture, TextureFilterProfile::ProfileMipMapped));
			}
			else
			{
				instance->set_texture(game->get_textures()->get("white.png", TextureFilterProfile::ProfileNearest));
			}
			instance->set_material(m.material);
			instance->set_program(game->get_shaders()->get_program("sc_texture.vsh", "sc_texture.fsh"));
			instance->transform = m.transform;
		}

		res->bb = model.create_aabb();
		return std::move(res);
	}
}