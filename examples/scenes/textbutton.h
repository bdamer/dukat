#pragma once

#include <functional>
#include <dukat/dukat.h>

namespace dukat
{
	class TextButton : public UIControl
	{
	private:
		TextMeshInstance* text_mesh;

	public:
		std::string primary_color;
		std::string highlight_color;
		std::string text;

		TextButton(TextMeshInstance* text_mesh) : UIControl(), 
			text_mesh(text_mesh), primary_color("white"), highlight_color("yellow") { }
		~TextButton(void) { }

		void set_text(const std::string& text);
		void set_focus(bool focus);
	};
}