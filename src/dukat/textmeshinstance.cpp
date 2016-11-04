#include "stdafx.h"
#include "textmeshinstance.h"
#include "textmeshbuilder.h"

namespace dukat
{
	void TextMeshInstance::set_text(const std::string& text)
	{
		if (this->text == text)
			return;

		this->text = text;
		TextMeshBuilder mb;
		mb.rebuild_text_mesh2(get_mesh(), text, width, height);
	}

	void TextMeshInstance::set_size(float size)
	{
		transform.scale = { size, size, size };
		transform.update();
	}

	void TextMeshInstance::set_color(const Color& color)
	{
		Material m = get_material();
		m.ambient = color;
		set_material(m);
	}

	Color TextMeshInstance::get_color(void)
	{
		Material m = get_material();
		return m.ambient;
	}
}