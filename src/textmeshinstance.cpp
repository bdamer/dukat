#include "stdafx.h"
#include <dukat/textmeshinstance.h>
#include <dukat/textmeshbuilder.h>

namespace dukat
{
	TextMeshInstance::TextMeshInstance(std::unique_ptr<MeshData> text_mesh, float yorientation)
		: halign(Align::Left), valign(Align::Bottom), yorientation(yorientation)
	{
		this->text_mesh = std::move(text_mesh);
		set_mesh(this->text_mesh.get());
	}

	void TextMeshInstance::set_text(const std::string& text)
	{
		if (this->text == text)
			return;

		this->text = text;
		TextMeshBuilder mb;
		mb.rebuild_text_mesh(get_mesh(), text, width, height);
		update(0.0f);
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

	void TextMeshInstance::set_size(float size)
	{
		transform.scale = { size, size, size };
	}

	void TextMeshInstance::update(float delta) 
	{
		float x_offset;
		float y_offset;

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