#include "stdafx.h"
#include "textbutton.h"

namespace dukat
{
	void TextButton::set_text(const std::string& text)
	{
		this->text = text;
		std::stringstream ss;
		ss << "<#" << (is_focus() ? highlight_color : primary_color) << ">" << text << "</>";
		text_mesh->set_text(ss.str());
	}

	void TextButton::set_focus(bool focus)
	{
		UIControl::set_focus(focus);
		set_text(text); // update text
	}
}