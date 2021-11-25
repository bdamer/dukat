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
		static const auto max_length = 1024;
		uint32_t simple_hash(const std::string& name) const;
		BitmapFont* font;

		// attempts to parse color from tag
		bool parse_color(size_t& i, const std::string& text, Color& color) const;

	public:
		// Creates a new text mesh builder for a given font.
		TextMeshBuilder(BitmapFont* font) : font(font) { }
		~TextMeshBuilder(void) { }
		std::unique_ptr<MeshData> build_text_mesh(const std::string& text = "") const;
		void rebuild_text_mesh(MeshData* mesh, const std::string& text, const float char_width, const float line_height, float& width, float& height) const;
	};
}