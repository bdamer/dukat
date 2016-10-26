#pragma once

#include <string>
#include "meshinstance.h"

namespace dukat
{
	class TextMeshInstance : public MeshInstance
	{
	private:
		std::string text;

	public:
		TextMeshInstance(void) { }
		~TextMeshInstance(void) { }

		void set_text(const std::string& text);
		std::string get_text(void) const { return text; }

		void set_size(float size);
		void set_color(const Color& color);
		Color get_color(void);
	};
}