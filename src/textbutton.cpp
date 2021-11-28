#include "stdafx.h"
#include <dukat/textbutton.h>
#include <dukat/textmeshinstance.h>

namespace dukat
{
	void TextButton::set_text(const std::string& text)
	{
		this->text = text;
		text_mesh->set_text(text);
		text_mesh->update(); // force update of mesh
	}

	void TextButton::set_focus(bool focus)
	{
		UIControl::set_focus(focus);
		text_mesh->set_ambient(is_focus() ? highlight_color : primary_color);
	}
}