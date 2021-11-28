#pragma once

#include <memory>
#include <string>

namespace dukat
{
	class MeshData;
	struct Color;
	class BitmapFont;

	// Utility class to generate meshes to render text based on a sprite map.
	class TextMeshBuilder
	{
	private:
		// Max length in characters
		BitmapFont* font;
		float char_width;
		float line_height;
		float max_line_width;

		uint32_t simple_hash(const std::string& name) const;
		// attempts to parse color from tag
		bool parse_color(size_t& i, const std::string& text, Color& color) const;
		std::string add_line_breaks(const std::string& text) const;

	public:
		// Creates a new text mesh builder for a given font.
		TextMeshBuilder(BitmapFont* font, float char_width, float line_height, float max_line_width) 
			: font(font), char_width(char_width), line_height(line_height), max_line_width(max_line_width) { }
		~TextMeshBuilder(void) { }
		
		void rebuild_text_mesh(MeshData* mesh, const std::string& text, float& width, float& height) const;
	};
}