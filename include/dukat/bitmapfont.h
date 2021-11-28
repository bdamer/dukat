#pragma once

#include <array>
#include <dukat/texture.h>

namespace dukat
{
	// A glyph in texture space.
	struct BitmapGlyph
	{
		GLfloat x;
		GLfloat y;
		GLfloat width;
		GLfloat height;
		GLfloat x_advance;

		BitmapGlyph(void) : x(0.0f), y(0.0f), width(0.0f), height(0.0f), x_advance(0.0f) { }
	};

	// Represents a bitmap font
	// For now, the following limitations apply:
	// * Supports only 1 page
	// * Kerning pairs are not supported
	// * Texture data is always assumed to come from RGB channels
	class BitmapFont
	{
	private:
		std::array<BitmapGlyph, 256> glyphs;
		std::unique_ptr<Texture> texture;

	public:
		const std::string name;
		const std::string font_file;
		const float size; // base size in pixels

		BitmapFont(const std::string& name, const std::string& font_file, const float size, std::unique_ptr<Texture>& texture)
			: name(name), font_file(font_file), size(size) { this->texture = std::move(texture); }
		~BitmapFont(void) { }

		Texture* get_texture(void) const { return texture.get(); }
		void set_glyph(char c, const BitmapGlyph& glyph) { glyphs[128 + static_cast<int>(c)] = glyph; }
		const BitmapGlyph& get_glyph(char c) const { return glyphs[128 + static_cast<int>(c)]; }
	};

	std::unique_ptr<BitmapFont> load_font(const std::string& font_file, TextureFilterProfile filter_profile);
}