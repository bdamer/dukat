#pragma once

#include <string>
#include "meshinstance.h"

namespace dukat
{
	class TextMeshInstance : public MeshInstance
	{
	private:
		std::string text;
		// unscaled width and height
		float width, height; 

	public:
		TextMeshInstance(void) { }
		~TextMeshInstance(void) { }

		void set_text(const std::string& text);
		std::string get_text(void) const { return text; }
		void set_color(const Color& color);
		Color get_color(void);

		void set_size(float size);
		float get_width(void) const { return width * transform.scale.x; }
		float get_height(void) const { return height * transform.scale.x; }
	};
}