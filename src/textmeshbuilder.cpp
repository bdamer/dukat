#include "stdafx.h"
#include <dukat/textmeshbuilder.h>
#include <dukat/buffers.h>
#include <dukat/log.h>
#include <dukat/meshdata.h>
#include <dukat/renderer.h>
#include <dukat/vertextypes2.h>
#include <dukat/vertextypes3.h>
#include <map>

namespace dukat
{
	static constexpr Color default_color{ 1.0f, 1.0f, 1.0f, 1.0f };

	uint32_t TextMeshBuilder::simple_hash(const std::string& name) const
	{
		uint32_t id = 0;
		for (const auto& it : name)
			id = (id << 1) + static_cast<Uint32>(it);
		return id;
	}

	std::unique_ptr<MeshData> TextMeshBuilder::build_text_mesh(const std::string& text) const
	{
		std::vector<VertexAttribute> attr;
		attr.push_back(dukat::VertexAttribute(Renderer::at_pos, 2));
		attr.push_back(dukat::VertexAttribute(Renderer::at_color, 4));
		attr.push_back(dukat::VertexAttribute(Renderer::at_texcoord, 2));
		auto mesh = std::make_unique<MeshData>(GL_TRIANGLES, max_length * 6, 0, attr);
		float width, height;
		rebuild_text_mesh(mesh.get(), text, 1.0f, 1.0f, width, height);
		return mesh;
	}

	bool TextMeshBuilder::parse_color(size_t& i, const std::string& text, Color& color) const
	{
		i++;
		if (text[i] == '#') // start tag
		{
			const auto hex = text.substr(++i, 6);
			try
			{
				color = color_rgb(std::stoul(hex, nullptr, 16));
			}
			catch (const std::exception& e)
			{
				log->warn("Failed to parse color: {}", hex, e.what());
			}
			i += 6;
			return true;
		}
		else if (text[i] == '/') // end tag
		{
			i++; // skip character
			color = default_color;
			return true;
		}
		else
		{
			// no tag
			i--;
			return false;
		}
	}

	void TextMeshBuilder::rebuild_text_mesh(MeshData* mesh, const std::string& text, 
		const float char_width, const float line_height, float& width, float& height) const
	{
		assert(text.length() <= max_length);

		// build up vertices
		auto x = 0.0f, y = 0.0f;
		const auto tw = 1.0f / static_cast<float>(cols);
		const auto th = 1.0f / static_cast<float>(rows);
		auto max_x = 0.0f;
		std::vector<Vertex2PCT> verts;
		auto color = default_color;
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
				y += line_height;
				x = 0.0f;
				continue;
			}

			// Create vertex for this character
			auto u = static_cast<float>((c - 32) % 16) * tw;
			auto v = static_cast<float>((c - 32) / 16) * th;
			verts.push_back({ x, y, color.r, color.g, color.b, color.a, u, v }); // top-left
			verts.push_back({ x, y + 1.0f, color.r, color.g, color.b, color.a, u, v + th }); // bottom-left
			verts.push_back({ x + 1.0f, y, color.r, color.g, color.b, color.a, u + tw, v }); // top-right
			verts.push_back({ x + 1.0f, y, color.r, color.g, color.b, color.a, u + tw, v }); // top-right
			verts.push_back({ x, y + 1.0f, color.r, color.g, color.b, color.a, u, v + th }); // bottom-left
			verts.push_back({ x + 1.0f, y + 1.0f, color.r, color.g, color.b, color.a, u + tw, v + th }); // bottom-right

			max_x = std::max(x, max_x);
			x += char_width;
		}

		width = max_x + char_width;
		height = y + line_height;
		mesh->set_vertices(reinterpret_cast<GLfloat*>(verts.data()), static_cast<int>(verts.size()));
	}
}