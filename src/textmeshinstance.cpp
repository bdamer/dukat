#include "stdafx.h"
#include <dukat/textmeshinstance.h>
#include <dukat/textmeshbuilder.h>

namespace dukat
{
	TextMeshInstance::TextMeshInstance(BitmapFont* font, float yorientation)
		: text(""), char_width(1.0f), line_height(1.2f), max_line_width(0.0f),
		halign(Align::Left), valign(Align::Bottom), yorientation(yorientation), 
		num_vertices(0), font(font), scroll_delay(0.0f), 
		scroll_accumulator(0.0f), scroll_callback(nullptr), rebuild_required(false)
	{
		create_mesh();
		set_texture(font->get_texture());
	}

	void TextMeshInstance::set_text(const std::string& text)
	{
		if (this->text == text)
			return;
		assert(text.length() <= max_length);
		this->text = text;
		rebuild_required = true;
	}

	void TextMeshInstance::set_text_scroll(const std::string& text, float delay, const std::function<void(void)>& callback)
	{
		scroll_delay = delay;
		scroll_accumulator = 0.0f;
		scroll_callback = callback;

		set_text(text);
	}

	void TextMeshInstance::set_alpha(float alpha)
	{
		auto mat = get_material();
		mat.ambient.a = alpha;
		set_material(mat);
	}

	float TextMeshInstance::get_alpha(void) const
	{
		auto mat = get_material();
		return mat.ambient.a;
	}

	void TextMeshInstance::set_font(BitmapFont* font)
	{
		this->font = font;
		set_texture(font->get_texture());
		rebuild_required = true;
	}

	void TextMeshInstance::rebuild(void)
	{
		TextMeshBuilder mb(font, char_width, line_height, max_line_width);
		mb.rebuild_text_mesh(get_mesh(), text, width, height);

		// if scroll is active, reset number of characters on screen
		if (scroll_delay > 0.0f)
		{
			auto md = this->get_mesh();
			num_vertices = md->vertex_count();
			md->set_vertex_count(vertices_per_char);
		}

		rebuild_required = false;
	}

	void TextMeshInstance::create_mesh(void)
	{
		std::vector<VertexAttribute> attr;
		attr.push_back(dukat::VertexAttribute(Renderer::at_pos, 2));
		attr.push_back(dukat::VertexAttribute(Renderer::at_color, 4));
		attr.push_back(dukat::VertexAttribute(Renderer::at_texcoord, 2));
		this->text_mesh = std::make_unique<MeshData>(GL_TRIANGLES, max_length * 6, 0, attr);
		set_mesh(this->text_mesh.get());
	}

	void TextMeshInstance::update_scroll(float delta)
	{
		if (scroll_delay > 0.0f)
		{
			scroll_accumulator += delta;

			if (scroll_accumulator >= scroll_delay)
			{
				scroll_accumulator -= scroll_delay;
				auto md = this->get_mesh();
				const auto new_vc = md->vertex_count() + vertices_per_char;
				md->set_vertex_count(new_vc);

				if (new_vc >= num_vertices)
				{
					scroll_delay = 0.0f; // we're done with delayed print

					if (scroll_callback)
						scroll_callback();
				}
			}
		}
	}

	void TextMeshInstance::update(float delta)
	{
		if (rebuild_required)
			rebuild();

		update_scroll(delta);

		auto x_offset = 0.0f;
		auto y_offset = 0.0f;

		switch (halign)
		{
		case Left:
			x_offset = 0.0f;
			break;
		case Center:
			x_offset = -0.5f * get_width();
			break;
		case Right:
			x_offset = -get_width();
			break;
		}

		switch (valign)
		{
		case Bottom:
			y_offset = 0.0f;
			break;
		case Center:
			y_offset = yorientation * 0.5f * get_height();
			break;
		case Top:
			y_offset = yorientation * get_height();
			break;
		}

		// temporarily shift position by offset for alignment.
		auto tmp_pos = this->transform.position;
		this->transform.position.x += x_offset;
		this->transform.position.y += y_offset;
		this->transform.update();
		this->transform.position = tmp_pos;
	}
}