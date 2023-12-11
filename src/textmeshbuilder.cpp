#include "stdafx.h"
#include <dukat/textmeshbuilder.h>
#include <dukat/bitmapfont.h>
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

	std::string TextMeshBuilder::add_line_breaks(const std::string& text) const
	{
		std::stringstream ss;

		auto processed = 0;
		while (true)
		{
			auto last_space = -1;
			auto cur_width = 0.0f;

			auto line_break = false;
			for (auto i = 0u; (processed + i) < text.length(); i++)
			{
				const auto c = text[processed + i];
				if (c == ' ') // remember last space
					last_space = processed + i;
				else if (c == '\n')
				{
					ss << text.substr(processed, i) << std::endl;
					processed += i + 1; // acount for newline
					line_break = true;
					break;
				}

				const auto& glyph = font->get_glyph(c);
				cur_width += glyph.x_advance;
				if (cur_width > max_line_width)
				{
					assert(last_space >= 0);
					// need to break
					ss << text.substr(processed, last_space - processed) << std::endl;
					processed = last_space + 1; // acount for the space
					line_break = true;
					break;
				}
			}

			// handle last chunk
			if (!line_break)
			{
				ss << text.substr(processed);
				break;
			}
		}

		return ss.str();
	}

	void TextMeshBuilder::rebuild_text_mesh(MeshData* mesh, const std::string& text, float& width, float& height) const
	{
		const auto processed_text = max_line_width > 0.0f ? add_line_breaks(text) : text;

		auto tex = font->get_texture();

		// build up vertices
		auto x = 0.0f, y = 0.0f;
		auto max_x = 0.0f;
		std::vector<Vertex2PCT> verts;
		auto color = default_color;
		for (std::size_t i = 0u; i < processed_text.length(); i++)
		{
			char c = processed_text[i];
			if (c == '<' && (i < processed_text.length() - 1))
			{
				if (parse_color(i, processed_text, color))
					continue;
			}
			else if (c == '\n')
			{
				y += line_height;
				x = 0.0f;
				continue;
			}

			const auto& glyph = font->get_glyph(c);

			// Texture coordinates
			const auto u = glyph.x;
			const auto v = glyph.y;
			const auto tw = glyph.width;
			const auto th = glyph.height;

			// Vertex size based texture size 
			const auto w = (tw * tex->w) / font->size;
			const auto h = 1.0f;

			verts.push_back({ x, y, color.r, color.g, color.b, color.a, u, v }); // top-left
			verts.push_back({ x, y + h, color.r, color.g, color.b, color.a, u, v + th }); // bottom-left
			verts.push_back({ x + w, y, color.r, color.g, color.b, color.a, u + tw, v }); // top-right
			verts.push_back({ x + w, y, color.r, color.g, color.b, color.a, u + tw, v }); // top-right
			verts.push_back({ x, y + h, color.r, color.g, color.b, color.a, u, v + th }); // bottom-left
			verts.push_back({ x + w, y + h, color.r, color.g, color.b, color.a, u + tw, v + th }); // bottom-right

			max_x = std::max(x, max_x);
			x += glyph.x_advance * char_width;
		}

		width = max_x + char_width;
		height = y + line_height;
		mesh->set_vertices(reinterpret_cast<GLfloat*>(verts.data()), static_cast<int>(verts.size()));
	}
}