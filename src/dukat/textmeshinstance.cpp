#include "stdafx.h"
#include "textmeshinstance.h"
#include "textmeshbuilder.h"

namespace dukat
{
	TextMeshInstance::TextMeshInstance(std::unique_ptr<MeshData> text_mesh) : align(Align::Left)
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
		switch (align)
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
		// temporarily shift X position by offset for alignment.
		auto tmp_x = this->transform.position.x;
		this->transform.position.x += x_offset;
		this->transform.update(); 
		this->transform.position.x = tmp_x;
	}
}