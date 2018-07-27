#include "stdafx.h"

#include <cassert>

#include "log.h"
#include "material.h"
#include "mathutil.h"
#include "model3.h"
#include "ms3dmodel.h"

namespace dukat
{
	std::unique_ptr<Model3> MS3DModel::convert(void)
	{
		// Convert from left-handed to right-handed coordinate system.
		// align default orientation with Z axis. Since we're only rotating,
		// x axis coordinates will be flipped.
		Matrix4 m;
		m.setup_rotation(Vector3::unit_y, -pi_over_two);

		auto res = std::make_unique<Model3>();
		for (auto& mesh : meshes)
		{
			std::string name(mesh.name);

			auto& mat = materials[mesh.material];
			dukat::Material material;
			memcpy(&material.ambient, mat.ambient, sizeof(GLfloat) * 4);
			memcpy(&material.diffuse, mat.diffuse, sizeof(GLfloat) * 4);
			memcpy(&material.specular, mat.specular, sizeof(GLfloat) * 4);
			material.custom = { mat.shininess, 1.0f, 1.0f, 1.0f };

			// build up vertex buffer
			std::vector<GLushort> index_data;
			std::vector<Model3::Vertex> vertex_data;
			for (auto idx : mesh.indices)
			{
				auto& t = triangles[idx];
				for (auto i = 0; i < 3; i++)
				{
					assert(t.indicies[i] < static_cast<int16_t>(vertices.size()));
					auto vtmp = Vector3{vertices[t.indicies[i]].v[0], vertices[t.indicies[i]].v[1], vertices[t.indicies[i]].v[2]} * m;
					auto ntmp = Vector3{t.normals[i * 3], t.normals[i * 3 + 1], t.normals[i * 3 + 2]} * m;
					vertex_data.push_back({
						vtmp.x, vtmp.y, vtmp.z,	// POS
						ntmp.x, ntmp.y, ntmp.z, // NOR
						t.s[i], t.t[i] // TEX
					});
				}
			}

			res->add_mesh(name, material, mat.texture, Transform3{}, index_data, vertex_data);		
		}

		return std::move(res);
	}

	std::istream& operator>>(std::istream& is, MS3DModel& m)
	{
		// Load header
		is.read(reinterpret_cast<char*>(&m.header.id), sizeof(uint8_t) * 4);
		is.seekg(6, std::ios_base::cur);
		auto res = strcmp(m.header.id, MS3DModel::ms3d_id);
		if (res != 0)
		{
			throw std::runtime_error("Not a MS3D file!");
		}
		is.read(reinterpret_cast<char*>(&m.header.version), sizeof(uint32_t));
		if (m.header.version != MS3DModel::ms3d_version)
		{
			throw std::runtime_error("Invalid MS3D file.");
		}

		// Load vertex data
		m.vertices.resize(0);
		is.read(reinterpret_cast<char*>(&m.header.vertices), sizeof(uint16_t));
		for (auto i = 0; i < m.header.vertices; i++)
		{
			MS3DModel::Vertex vertex;
			is.read(reinterpret_cast<char*>(&vertex.flags), sizeof(uint8_t));
			is.read(reinterpret_cast<char*>(&vertex.v), sizeof(float) * 3);
			is.read(reinterpret_cast<char*>(&vertex.boneId), sizeof(uint8_t));
			is.read(reinterpret_cast<char*>(&vertex.ref_count), sizeof(uint8_t));
			m.vertices.push_back(vertex);
		}

		// Load polygon data
		m.triangles.resize(0);
		is.read(reinterpret_cast<char*>(&m.header.polygons), sizeof(uint16_t));
		for (auto i = 0; i < m.header.polygons; i++)
		{
			MS3DModel::Triangle triangle;
			is.read(reinterpret_cast<char*>(&triangle.flags), sizeof(uint16_t));
			is.read(reinterpret_cast<char*>(&triangle.indicies), sizeof(uint16_t) * 3);
			is.read(reinterpret_cast<char*>(&triangle.normals), sizeof(float) * 9);
			is.read(reinterpret_cast<char*>(&triangle.s), sizeof(float) * 3);
			is.read(reinterpret_cast<char*>(&triangle.t), sizeof(float) * 3);
			is.read(reinterpret_cast<char*>(&triangle.smoothing_group), sizeof(uint8_t));
			is.read(reinterpret_cast<char*>(&triangle.group_index), sizeof(uint8_t));
			m.triangles.push_back(triangle);
		}

		// Load mesh data
		m.meshes.resize(0);
		is.read(reinterpret_cast<char*>(&m.header.meshes), sizeof(uint16_t));
		for (auto i = 0; i < m.header.meshes; i++)
		{
			MS3DModel::Mesh mesh;
			is.read(reinterpret_cast<char*>(&mesh.flags), sizeof(uint8_t));
			is.read(reinterpret_cast<char*>(&mesh.name), sizeof(char) * 32);
			is.read(reinterpret_cast<char*>(&mesh.count), sizeof(uint16_t));
			// read triangles and material
			mesh.indices.resize(mesh.count);
			is.read(reinterpret_cast<char*>(mesh.indices.data()), sizeof(uint16_t) * mesh.count);
			is.read(reinterpret_cast<char*>(&mesh.material), sizeof(uint8_t));
			m.meshes.push_back(mesh);
		}

		// Load material data
		m.materials.resize(0);
		is.read(reinterpret_cast<char*>(&m.header.materials), sizeof(uint16_t));
		for (auto i = 0; i < m.header.materials; i++)
		{
			MS3DModel::Material material;
			is.read(reinterpret_cast<char*>(&material.name), sizeof(char) * 32);
			is.read(reinterpret_cast<char*>(&material.ambient), sizeof(float) * 4);
			is.read(reinterpret_cast<char*>(&material.diffuse), sizeof(float) * 4);
			is.read(reinterpret_cast<char*>(&material.specular), sizeof(float) * 4);
			is.read(reinterpret_cast<char*>(&material.emissive), sizeof(float) * 4);
			is.read(reinterpret_cast<char*>(&material.shininess), sizeof(float));
			is.read(reinterpret_cast<char*>(&material.transparency), sizeof(float));
			is.read(reinterpret_cast<char*>(&material.mode), sizeof(uint8_t));
			is.read(reinterpret_cast<char*>(&material.texture), sizeof(char) * 128);
			is.read(reinterpret_cast<char*>(&material.alphamap), sizeof(char) * 128);
			m.materials.push_back(material);
		}

		log->debug("Loaded MS3D model: {} vertices, {} polygons, {} meshes, {} materials.",
			m.header.vertices, m.header.polygons, m.header.meshes, m.header.materials);

		return is;
	}
}
