#include "stdafx.h"
#include "meshbuilder2.h"
#include "meshdata.h"
#include "vertextypes3.h"
#include "geometry.h"
#include "buffers.h"
#include "renderer.h"
#include "mathutil.h"
#include "vector3.h"

namespace dukat
{
	std::unique_ptr<MeshData> MeshBuilder2::build_axis(void)
	{
		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3, offsetof(VertexPosCol, pos)));
		attr.push_back(VertexAttribute(Renderer::at_color, 4, offsetof(VertexPosCol, col)));

		VertexPosCol vertices[4] = {
			0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
		};

		auto res = std::make_unique<MeshData>(GL_LINES, 4, 0, attr);
		res->set_vertices(reinterpret_cast<GLfloat*>(vertices));
		return res;
	}

	std::unique_ptr<MeshData> MeshBuilder2::build_points(const std::vector<Vector3>& points, const Color& color)
	{
		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3, offsetof(VertexPosCol, pos)));
		attr.push_back(VertexAttribute(Renderer::at_color, 4, offsetof(VertexPosCol, col)));

		std::vector<VertexPosCol> vertices;
		for (const auto& it : points)
		{
			vertices.push_back({ it.x, it.y, it.z, color.r, color.g, color.b, color.a });
		}

		auto res = std::make_unique<MeshData>(GL_POINTS, (unsigned int)vertices.size(), 0, attr);
		res->set_vertices(reinterpret_cast<GLfloat*>(vertices.data()));
		return res;
	}

	std::unique_ptr<MeshData> MeshBuilder2::build_triangle(void)
	{
		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3, offsetof(VertexPosCol, pos)));
		attr.push_back(VertexAttribute(Renderer::at_color, 4, offsetof(VertexPosCol, col)));

		VertexPosCol vertices[3] = {
			-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
		};

		auto res = std::make_unique<MeshData>(GL_TRIANGLES, 3, 0, attr);
		res->set_vertices(reinterpret_cast<GLfloat*>(vertices));
		return res;
	}

	std::unique_ptr<MeshData> MeshBuilder2::build_quad(void)
	{
		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3, offsetof(VertexPosCol, pos)));
		attr.push_back(VertexAttribute(Renderer::at_color, 4, offsetof(VertexPosCol, col)));

		VertexPosCol vertices[4] = {
			-1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f
		};

		auto res = std::make_unique<MeshData>(GL_TRIANGLE_STRIP, 4, 0, attr);
		res->set_vertices(reinterpret_cast<GLfloat*>(vertices));
		return res;
	}

	std::unique_ptr<MeshData> MeshBuilder2::build_textured_quad(const std::array<float, 4>& uv)
	{
		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3));
		attr.push_back(VertexAttribute(Renderer::at_texcoord, 2));

		VertexPosTex verts[4] = {
			-1.0f,  1.0f, 0.0f, uv[0], uv[3], // top-left
			-1.0f, -1.0f, 0.0f, uv[0], uv[1], // bottom-left
			1.0f,  1.0f, 0.0f, uv[2], uv[3], // top-right
			1.0f, -1.0f, 0.0f, uv[2], uv[1]  // bottom-right
		};

		auto res = std::make_unique<MeshData>(GL_TRIANGLE_STRIP, 4, 0, attr);
		res->set_vertices(reinterpret_cast<GLfloat*>(verts));
		return res;
	}

	std::unique_ptr<MeshData> MeshBuilder2::build_circle(int segments)
	{
		assert(segments > 2);

		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3, offsetof(VertexPosCol, pos)));
		attr.push_back(VertexAttribute(Renderer::at_color, 4, offsetof(VertexPosCol, col)));

		std::vector<VertexPosCol> vertices;

		// angle per segment
		float theta = two_pi / (float)segments;
		// calculate the tangential factor 
		float tan_factor = std::tan(theta);
		// calculate the radial factor 
		float rad_factor = std::cos(theta);

		// we start at angle = 0 
		float x = 1.0f;
		float y = 0;

		for (int i = 0; i <= segments; i++)
		{
			vertices.push_back({ x, y, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f });

			// calculate the tangential vector 
			// remember, the radial vector is (x, y) 
			// to get the tangential vector we flip those coordinates and negate one of them 
			float tx = -y;
			float ty = x;

			// add the tangential vector 
			x += tx * tan_factor;
			y += ty * tan_factor;

			// correct using the radial factor 
			x *= rad_factor;
			y *= rad_factor;
		}

		auto res = std::make_unique<MeshData>(GL_LINE_STRIP, (unsigned int)vertices.size(), 0, attr);
		res->set_vertices(reinterpret_cast<GLfloat*>(vertices.data()));
		return res;
	}

	std::unique_ptr<MeshData> MeshBuilder2::build_filled_circle(int segments)
	{
		assert(segments > 2);

		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3, offsetof(VertexPosCol, pos)));
		attr.push_back(VertexAttribute(Renderer::at_color, 4, offsetof(VertexPosCol, col)));

		std::vector<VertexPosCol> vertices;
		// add center position
		vertices.push_back({ 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f });

		// angle per segment
		float theta = two_pi / (float)segments;
		// calculate the tangential factor 
		float tan_factor = std::tan(theta);
		// calculate the radial factor 
		float rad_factor = std::cos(theta);
		
		// we start at angle = 0 
		float x = 1.0f;
		float y = 0;

		for (int i = 0; i <= segments; i++)
		{
			vertices.push_back({ x, y, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f });

			// calculate the tangential vector 
			// remember, the radial vector is (x, y) 
			// to get the tangential vector we flip those coordinates and negate one of them 
			float tx = -y;
			float ty = x;

			// add the tangential vector 
			x += tx * tan_factor;
			y += ty * tan_factor;

			// correct using the radial factor 
			x *= rad_factor;
			y *= rad_factor;
		}

		auto res = std::make_unique<MeshData>(GL_TRIANGLE_FAN, (unsigned int)vertices.size(), 0, attr);
		res->set_vertices(reinterpret_cast<GLfloat*>(vertices.data()));
		return res;
	}
}