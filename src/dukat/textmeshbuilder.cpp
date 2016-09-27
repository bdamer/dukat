#include "stdafx.h"
#include "textmeshbuilder.h"
#include "buffers.h"
#include "mesh.h"
#include "renderer.h"
#include "vertextypes2.h"
#include "vertextypes3.h"

namespace dukat
{
	std::unique_ptr<Mesh> TextMeshBuilder::build_text_mesh2(const std::string& text) const
	{
		std::vector<VertexAttribute> attr;
		attr.push_back(dukat::VertexAttribute(Renderer::at_pos, 2));
		attr.push_back(dukat::VertexAttribute(Renderer::at_texcoord, 2));
		auto mesh = std::make_unique<Mesh>(GL_TRIANGLES, max_length * 6, 0, attr);
		rebuild_text_mesh2(mesh.get(), text);
		return std::move(mesh);
	}

	std::unique_ptr<Mesh> TextMeshBuilder::build_text_mesh3(const std::string& text) const
	{
		std::vector<VertexAttribute> attr;
		attr.push_back(dukat::VertexAttribute(Renderer::at_pos, 3));
		attr.push_back(dukat::VertexAttribute(Renderer::at_texcoord, 2));
		auto mesh = std::make_unique<Mesh>(GL_TRIANGLES, max_length * 6, 0, attr);
		rebuild_text_mesh3(mesh.get(), text);
		return std::move(mesh);
	}

	void TextMeshBuilder::rebuild_text_mesh2(Mesh* mesh, const std::string& text) const
	{
		assert(text.length() <= max_length);

		// build up vertices
		float x = 0.0f;
		float y = 0.0f;
		float tw = 1.0f / (float)cols;
		float th = 1.0f / (float)rows;
		std::vector<TexturedVertex2> verts;
		for (auto& it : text)
		{
			if (it == '\n')
			{
				y += 1.0f;
				x = 0.0f;
				continue;
			}

			float u = (float)((it - 32) % 16) * tw;
			float v = (float)((it - 32) / 16) * th;
			verts.push_back({ x, y, u, v }); // top-left
			verts.push_back({ x, y + 1.0f, u, v + th }); // bottom-left
			verts.push_back({ x + 1.0f, y, u + tw, v }); // top-right
			verts.push_back({ x + 1.0f, y, u + tw, v }); // top-right
			verts.push_back({ x, y + 1.0f, u, v + th }); // bottom-left
			verts.push_back({ x + 1.0f, y + 1.0f, u + tw, v + th }); // bottom-right
			x += 1.0f;
		}

		mesh->set_vertices(reinterpret_cast<GLfloat*>(verts.data()), (int)verts.size());
	}

	void TextMeshBuilder::rebuild_text_mesh3(Mesh* mesh, const std::string& text) const
	{
		assert(text.length() <= max_length);

		// build up vertices
		float x = 0.0f;
		float y = 0.0f;
		float tw = 1.0f / (float)cols;
		float th = 1.0f / (float)rows;
		std::vector<dukat::VertexPosTex> verts;
		for (auto& it : text)
		{
			if (it == '\n')
			{
				y -= 1.0f;
				x = 0.0f;
				continue;
			}

			float u = (float)((it - 32) % 16) * tw;
			float v = (float)((it - 32) / 16) * th;
			verts.push_back({ x, y, 0.0f, u, 1.0f - v }); // top-left
			verts.push_back({ x, y - 1.0f,	0.0f, u, 1.0f - (v + th) }); // bottom-left
			verts.push_back({ x + 1.0f, y, 0.0f, u + tw, 1.0f - v }); // top-right
			verts.push_back({ x + 1.0f, y, 0.0f, u + tw, 1.0f - v }); // top-right
			verts.push_back({ x, y - 1.0f,	0.0f, u, 1.0f - (v + th) }); // bottom-left
			verts.push_back({ x + 1.0f, y - 1.0f,	0.0f, u + tw, 1.0f - (v + th) }); // bottom-right
			x += 1.0f;
		}

		mesh->set_vertices(reinterpret_cast<GLfloat*>(verts.data()), (int)verts.size());
	}
}