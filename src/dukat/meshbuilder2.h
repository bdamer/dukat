#pragma once

#include <array>
#include <memory>
#include <vector>

namespace dukat
{
	class MeshData;
	class Vector3;
	struct Color;

	// Utility class the generate 2D meshes. Unless otherwise noted, this class
	// will generate unit-sized meshes (i.e., sphere with radius = 1, quad with edge length = 1)
	class MeshBuilder2
	{
	public:
		MeshBuilder2(void) { }
		~MeshBuilder2(void) { }

		// Creates a new coordinate axis mesh.
		std::unique_ptr<MeshData> build_axis(void);
		// Creates a new mesh for a set of points.
		std::unique_ptr<MeshData> build_points(const std::vector<Vector3>& points, const Color& color);
		// Creates a new quad mesh.
		std::unique_ptr<MeshData> build_quad(void);
		// Creates a new quad mesh with a given set of texture coordinates.
		std::unique_ptr<MeshData> build_textured_quad(const std::array<float, 4>& uv = { 0.0f, 0.0f, 1.0f, 1.0f });
		// Creates a new triangle mesh.
		std::unique_ptr<MeshData> build_triangle(void);
		// Creates a new circle mesh.
		std::unique_ptr<MeshData> build_circle(int segments);
	};
}