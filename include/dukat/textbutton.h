#pragma once

#include <functional>
#include "color.h"
#include "uicontrol.h"

namespace dukat
{
	class TextMeshInstance;

	class TextButton : public UIControl
	{
	private:
		TextMeshInstance* text_mesh;

	public:
		Color primary_color;
		Color highlight_color;
		std::string text;

		TextButton(TextMeshInstance* text_mesh) : UIControl(), 
			text_mesh(text_mesh), primary_color(Color{ 1.0f, 1.0f, 1.0f, 1.0f }), highlight_color(Color{ 0.98f, 0.95f, 0.02f, 1.0f }) { }
		~TextButton(void) { }

		void set_text(const std::string& text);
		void set_focus(bool focus);
	};
}