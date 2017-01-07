#include "stdafx.h"
#include "textmeshbuilder.h"
#include "buffers.h"
#include "mesh.h"
#include "renderer.h"
#include "vertextypes2.h"
#include "vertextypes3.h"
#include <map>

namespace dukat
{
	static std::map<Uint32, Color> color_map;

	Uint32 TextMeshBuilder::simple_hash(const std::string& name) const
	{
		Uint32 id = 0;
		for (const auto& it : name)
		{
			id = (id << 1) + (Uint32)it;
		}
		return id;
	}

	TextMeshBuilder::TextMeshBuilder(int cols, int rows) : cols(cols), rows(rows)
	{
		if (color_map.empty())
		{
			color_map[simple_hash("white")] = { 1.0f, 1.0f, 1.0f, 1.0f };
			color_map[simple_hash("yellow")] = { 251.0f / 255.0f, 243.0f / 255.0f, 5.0f / 255.0f, 1.0f };
			color_map[simple_hash("orange")] = { 255.0f / 255.0f, 100.0f / 255.0f, 3.0f / 255.0f, 1.0f };
			color_map[simple_hash("red")] = { 221.0f / 255.0f, 9.0f / 255.0f, 7.0f / 255.0f, 1.0f };
			color_map[simple_hash("magenta")] = { 242.0f / 255.0f, 8.0f / 255.0f, 132.0f / 255.0f, 1.0f };
			color_map[simple_hash("purple")] = { 71.0f / 255.0f, 0.0f / 255.0f, 165.0f / 255.0f, 1.0f };
			color_map[simple_hash("blue")] = { 0.0f / 255.0f, 0.0f / 255.0f, 211.0f / 255.0f, 1.0f };
			color_map[simple_hash("cyan")] = { 2.0f / 255.0f, 171.0f / 255.0f, 234.0f / 255.0f, 1.0f };
			color_map[simple_hash("green")] = { 31.0f / 255.0f, 183.0f / 255.0f, 20.0f / 255.0f, 1.0f };
			color_map[simple_hash("darkgreen")] = { 0.0f / 255.0f, 100.0f / 255.0f, 18.0f / 255.0f, 1.0f };
			color_map[simple_hash("brown")] = { 86.0f / 255.0f, 44.0f / 255.0f, 5.0f / 255.0f, 1.0f };
			color_map[simple_hash("tan")] = { 144.0f / 255.0f, 113.0f / 255.0f, 58.0f / 255.0f, 1.0f };
			color_map[simple_hash("lightgrey")] = { 192.0f / 255.0f, 192.0f / 255.0f, 192.0f / 255.0f, 1.0f };
			color_map[simple_hash("mediumgrey")] = { 128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 1.0f };
			color_map[simple_hash("darkgrey")] = { 64.0f / 255.0f, 64.0f / 255.0f, 64.0f / 255.0f, 1.0f };
			color_map[simple_hash("black")] = { 0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 1.0f };
		}
	}

	std::unique_ptr<Mesh> TextMeshBuilder::build_text_mesh(const std::string& text) const
	{
		std::vector<VertexAttribute> attr;
		attr.push_back(dukat::VertexAttribute(Renderer::at_pos, 2));
		attr.push_back(dukat::VertexAttribute(Renderer::at_color, 4));
		attr.push_back(dukat::VertexAttribute(Renderer::at_texcoord, 2));
		auto mesh = std::make_unique<Mesh>(GL_TRIANGLES, max_length * 6, 0, attr);
		float width, height;
		rebuild_text_mesh(mesh.get(), text, width, height);
		return std::move(mesh);
	}

	bool TextMeshBuilder::parse_color(size_t& i, const std::string& text, Color& color) const
	{
		i++;
		if (text[i] == '#') // start tag
		{
			int color_id = 0;
			while (++i < text.length() && text[i] != '>')
			{
				color_id = (color_id << 1) + text[i];
			}
			if (color_map.count(color_id))
			{
				color = color_map.at(color_id);
			}
			return true;
		}
		else if (text[i] == '/') // end tag
		{
			i++; // skip character
			color = color_map.at(simple_hash("white"));
			return true;
		}
		else
		{
			// no tag
			i--;
			return false;
		}
	}

	void TextMeshBuilder::rebuild_text_mesh(Mesh* mesh, const std::string& text, float& width, float& height) const
	{
		assert(text.length() <= max_length);

		// build up vertices
		float x = 0.0f, y = 0.0f;
		float tw = 1.0f / (float)cols, th = 1.0f / (float)rows;
		float max_x = 0.0f;
		std::vector<ColoredTexturedVertex2> verts;
		Color color = color_map.at(simple_hash("white"));
		for (size_t i = 0; i < text.length(); i++)
		{
			char c = text[i];
			if (c == '<' && (i < text.length() - 1))
			{
				if (parse_color(i, text, color))
					continue;
			}
			else if (c == '\n')
			{
				y += 1.0f;
				x = 0.0f;
				continue;
			}

			// Create vertex for this character
			float u = (float)((c - 32) % 16) * tw;
			float v = (float)((c - 32) / 16) * th;
			verts.push_back({ x, y, color.r, color.g, color.b, color.a, u, v }); // top-left
			verts.push_back({ x, y + 1.0f, color.r, color.g, color.b, color.a, u, v + th }); // bottom-left
			verts.push_back({ x + 1.0f, y, color.r, color.g, color.b, color.a, u + tw, v }); // top-right
			verts.push_back({ x + 1.0f, y, color.r, color.g, color.b, color.a, u + tw, v }); // top-right
			verts.push_back({ x, y + 1.0f, color.r, color.g, color.b, color.a, u, v + th }); // bottom-left
			verts.push_back({ x + 1.0f, y + 1.0f, color.r, color.g, color.b, color.a, u + tw, v + th }); // bottom-right

			max_x = std::max(x, max_x);
			x += 1.0f;
		}

		width = max_x + 1.0f;
		height = y + 1.0f;
		mesh->set_vertices(reinterpret_cast<GLfloat*>(verts.data()), (int)verts.size());
	}
}