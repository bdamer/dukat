#include "stdafx.h"
#include "textbutton.h"

namespace dukat
{
	void TextButton::set_text(const std::string& text)
	{
		this->text = text;
		std::stringstream ss;
		ss << "<#" << (in_focus ? highlight_color : primary_color) << ">" << text << "</>";
		text_mesh->set_text(ss.str());
	}

	void TextButton::gain_focus(void)
	{
		in_focus = true;
		set_text(text);
	}

	void TextButton::lose_focus(void)
	{
		in_focus = false;
		set_text(text);
	}

	void TextButton::trigger(void)
	{
		if (func) 
			func();
	}
}