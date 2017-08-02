#pragma once

#include <cstdint>
#include <memory>
#include <istream>
#include <string>
#include <vector>
#include "modelconverter.h"

namespace dukat
{
	class Model3;
	
	class MS3DModel : public ModelConverter
	{
	private:
		static constexpr const char* ms3d_id = "MS3D0";
		static constexpr const int32_t ms3d_version = 4;

		struct Header
		{
			char id[5];
			int32_t version;
			int16_t vertices;
			int16_t polygons;
			int16_t meshes;
			int16_t materials;
		};

		struct Vertex
		{
			uint8_t flags;
			float v[3];
			uint8_t boneId;
			uint8_t ref_count;
		};

		struct Triangle
		{
			int16_t flags; // not just uchar?
			int16_t indicies[3];
			float normals[9]; // 3x3 array
			float s[3]; // ???
			float t[3]; // texure coords?
			uint8_t smoothing_group;
			uint8_t group_index;
		};

		struct Mesh
		{
			uint8_t flags;
			char name[32];
			uint16_t count; // triangle count
			std::vector<uint16_t> indices; // triangle indices
			uint8_t material;
		};

		struct Material
		{
			char name[32];
			float ambient[4];
			float diffuse[4];
			float specular[4];
			float emissive[4];
			float shininess; // 0.0f - 128.0f 
			float transparency; // 0.0f - 1.0f 
			uint8_t mode; // 0, 1, 2 is unused now 
			char texture[128];
			char alphamap[128];
		};

		struct Joint
		{
			uint8_t flags;
			char name[32];
			char parent_nname[32];
			float rotation[3];
			float translation[3];
			int16_t num_rotation_keyframes;
			int16_t num_translation_keyframes;
		};

		struct Keyframe
		{
			float time;
			float parameter[3]; // ?
		};

		Header header;
		std::vector<Vertex> vertices;
		std::vector<Triangle> triangles;
		std::vector<Mesh> meshes;
		std::vector<Material> materials;

	public:
		MS3DModel(void) { };
		~MS3DModel(void) { };

		// Creates a frontier model from this model.
		std::unique_ptr<Model3> convert(void);
		// Reads MS3D model from stream.
		friend std::istream& operator>>(std::istream& is, MS3DModel& v);
	};
}