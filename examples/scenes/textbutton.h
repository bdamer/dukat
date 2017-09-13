#pragma once

#include <functional>
#include <dukat/dukat.h>

namespace dukat
{
	class TextButton : public UIControl
	{
	private:
		TextMeshInstance* text_mesh;
		bool in_focus;

	public:
		std::string primary_color;
		std::string highlight_color;
		std::string text;
		std::function<void(void)> func;

		TextButton(TextMeshInstance* text_mesh) : UIControl(), 
			text_mesh(text_mesh), primary_color("white"), highlight_color("yellow"), in_focus(false) { }

		~TextButton(void) { }

		void set_text(const std::string& text);

		// Event handlers
		void gain_focus(void);
		void lose_focus(void);
		void trigger(void);
	};
}