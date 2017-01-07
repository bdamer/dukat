#include "stdafx.h"
#include "textmeshinstance.h"
#include "textmeshbuilder.h"

namespace dukat
{
	TextMeshInstance::TextMeshInstance(std::unique_ptr<Mesh> text_mesh)
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
		transform.update();
	}
}