#include "stdafx.h"
#include <dukat/bitmapfont.h>
#include <dukat/log.h>
#include <dukat/surface.h>
#include <dukat/sysutil.h>
#include <dukat/texture.h>

namespace dukat
{
	// BMF Format based on: https://www.angelcode.com/products/bmfont/doc/file_format.html
	struct BMFHeader
	{
		char id[3];
		char version;
	};

	struct BMFInfo
	{
		int16_t font_size;
		uint8_t bit_field; // bit 0: smooth, bit 1 : unicode, bit 2 : italic, bit 3 : bold, bit 4 : fixedHeigth, bits 5 - 7 : reserved
		uint8_t char_set;
		uint16_t stretch_h;
		uint8_t aa;
		uint8_t padding_up;
		uint8_t padding_right;
		uint8_t padding_down;
		uint8_t padding_left;
		uint8_t spacing_h;
		uint8_t spacing_v;
		uint8_t outline;
	};

	struct BMFCommon
	{
		uint16_t line_height; // This is the distance in pixels between each line of text.
		uint16_t base; // The number of pixels from the absolute top of the line to the base of the characters.
		uint16_t scale_w; // The width of the texture, normally used to scale the x pos of the character image.
		uint16_t scale_h; // The height of the texture, normally used to scale the y pos of the character image.
		uint16_t pages; // The number of texture pages included in the font.
		uint8_t bit_field;	// Set to 1 if the monochrome characters have been packed into each of the texture channels.In this case alphaChnl describes what is stored in each channel.
		uint8_t alpha_chnl; // Set to 0 if the channel holds the glyph data, 1 if it holds the outline, 2 if it holds the glyphand the outline, 3 if its set to zero, and 4 if its set to one.
		uint8_t red_chnl; // Set to 0 if the channel holds the glyph data, 1 if it holds the outline, 2 if it holds the glyphand the outline, 3 if its set to zero, and 4 if its set to one.
		uint8_t green_chnl; // Set to 0 if the channel holds the glyph data, 1 if it holds the outline, 2 if it holds the glyphand the outline, 3 if its set to zero, and 4 if its set to one.
		uint8_t blue_chnl; // Set to 0 if the channel holds the glyph data, 1 if it holds the outline, 2 if it holds the glyphand the outline, 3 if its set to zero, and 4 if its set to one.
	};

	struct BMFChar
	{
		uint32_t id; // The character id.
		uint16_t x; // The left position of the character image in the texture.
		uint16_t y; // The top position of the character image in the texture.
		uint16_t width; // The width of the character image in the texture.
		uint16_t height; // The height of the character image in the texture.
		int16_t x_offset; // How much the current position should be offset when copying 
						   // the image from the texture to the screen.
		int16_t y_offset; // How much the current position should be offset when copying
						   // the image from the texture to the screen.
		int16_t x_advance; // How much the current position should be advanced after drawing the character.
		uint8_t page; // The texture page where the character image is found.
		uint8_t channel; // The texture channel where the character image is found(1 = blue, 2 = green, 4 = red, 8 = alpha, 15 = all channels).
	};

	struct BMFKerningPair
	{
		uint32_t first; // These fields are repeated until all kerning pairs have been described
		uint32_t second;
		int16_t	amount;
	};

	struct BMFFile
	{
		BMFHeader header;
		BMFInfo info;
		BMFCommon common;
		std::string font_name;
		std::vector<std::string> pages;
		std::vector<BMFChar> chars;
	};

	void load_bmf_file(std::istream& is, BMFFile& file)
	{
		// Validate header
		is.read(reinterpret_cast<char*>(&file.header), sizeof(BMFHeader));
		const auto magic = "BMF";
		if (std::memcmp(reinterpret_cast<const void*>(file.header.id), reinterpret_cast<const void*>(magic), 3))
			throw std::runtime_error("Invalid BMF file header.");
		if (file.header.version != 3)
			throw std::runtime_error("Invalid BMF file version.");

		// Read blocks
		while (is)
		{
			uint8_t block_type;
			uint32_t block_size;
			is.read(reinterpret_cast<char*>(&block_type), sizeof(uint8_t));
			is.read(reinterpret_cast<char*>(&block_size), sizeof(uint32_t));

			switch (block_type)
			{
			case 1: // INFO block
				is.read(reinterpret_cast<char*>(&file.info), sizeof(BMFInfo));
				std::getline(is, file.font_name, '\0');
				break;

			case 2: // COMMON block
				assert(block_size == 15);
				is.read(reinterpret_cast<char*>(&file.common.line_height), sizeof(uint16_t));
				is.read(reinterpret_cast<char*>(&file.common.base), sizeof(uint16_t));
				is.read(reinterpret_cast<char*>(&file.common.scale_w), sizeof(uint16_t));
				is.read(reinterpret_cast<char*>(&file.common.scale_h), sizeof(uint16_t));
				is.read(reinterpret_cast<char*>(&file.common.pages), sizeof(uint16_t));
				is.read(reinterpret_cast<char*>(&file.common.bit_field), sizeof(uint8_t));
				is.read(reinterpret_cast<char*>(&file.common.alpha_chnl), sizeof(uint8_t));
				is.read(reinterpret_cast<char*>(&file.common.red_chnl), sizeof(uint8_t));
				is.read(reinterpret_cast<char*>(&file.common.green_chnl), sizeof(uint8_t));
				is.read(reinterpret_cast<char*>(&file.common.blue_chnl), sizeof(uint8_t));
				break;

			case 3: // PAGES block
			{
				// read first page - all subsequent pages will have same length
				std::string page;
				std::getline(is, page, '\0');
				file.pages.push_back(page);
				auto num_pages = block_size / (page.length() + 1) - 1;
				while (num_pages-- > 0)
				{
					std::getline(is, page, '\0');
					file.pages.push_back(page);
				}
				break;
			}

			case 4: // CHARS block
			{
				BMFChar c;
				auto num_chars = block_size / sizeof(BMFChar);
				while (num_chars-- > 0)
				{
					is.read(reinterpret_cast<char*>(&c), sizeof(BMFChar));
					file.chars.push_back(c);
				}
				break;
			}

			case 5: // KERNING PAIRS block
				assert(false); // not supported
				break;
			}
		}
	}

	std::unique_ptr<BitmapFont> load_font(const std::string& font_file, TextureFilterProfile filter_profile)
	{
		log->info("Loading font: {}", font_file);
		std::fstream fs(font_file, std::fstream::in | std::fstream::binary);
		if (!fs)
			throw std::runtime_error("Could not open file: " + font_file);

		BMFFile file;
		load_bmf_file(fs, file);
		assert(file.pages.size() == 1);

		const auto texture_file = dir_name(font_file) + file.pages[0];
		log->debug("Loading font texture: {}", texture_file);
		auto surface = Surface::from_file(texture_file);
		// This assumes surface is already in 32 bit format
		auto texture = std::make_unique<Texture>(*surface, filter_profile);

		auto res = std::make_unique<BitmapFont>(file.font_name, font_file,
			static_cast<float>(file.info.font_size),
			texture
		);

		const auto tex_w = static_cast<float>(file.common.scale_w);
		const auto tex_h = static_cast<float>(file.common.scale_h);
		for (const auto& c : file.chars)
		{
			if (c.id >= 255)
				continue; // skip unused glyphs

			BitmapGlyph glyph;
			glyph.x = static_cast<float>(c.x) / tex_w;
			glyph.y = static_cast<float>(c.y) / tex_h;
			glyph.width = static_cast<float>(c.width) / tex_w;
			glyph.height = static_cast<float>(c.height) / tex_h;
			glyph.x_advance = static_cast<float>(c.x_advance) / static_cast<float>(file.info.font_size);

			const auto character = static_cast<char>(c.id & 0x000000ff);
			res->set_glyph(character, glyph);
		}

		return std::move(res);
	}

	void save_bmf_file(const BMFFile& file, std::ostream& os)
	{
		os.write(reinterpret_cast<const char*>(&file.header), sizeof(BMFHeader));

		// INFO
		uint8_t block_type = 1;
		os.write(reinterpret_cast<const char*>(&block_type), sizeof(uint8_t));
		uint32_t block_size = sizeof(BMFInfo) + file.font_name.length() + 1;
		os.write(reinterpret_cast<const char*>(&block_size), sizeof(uint32_t));
		os.write(reinterpret_cast<const char*>(&file.info), sizeof(BMFInfo));
		os.write(file.font_name.c_str(), static_cast<int>(file.font_name.length() + 1));

		// COMMON 2
		block_type = 2;
		os.write(reinterpret_cast<const char*>(&block_type), sizeof(uint8_t));
		block_size = 15;
		os.write(reinterpret_cast<const char*>(&block_size), sizeof(uint32_t));
		os.write(reinterpret_cast<const char*>(&file.common.line_height), sizeof(uint16_t));
		os.write(reinterpret_cast<const char*>(&file.common.base), sizeof(uint16_t));
		os.write(reinterpret_cast<const char*>(&file.common.scale_w), sizeof(uint16_t));
		os.write(reinterpret_cast<const char*>(&file.common.scale_h), sizeof(uint16_t));
		os.write(reinterpret_cast<const char*>(&file.common.pages), sizeof(uint16_t));
		os.write(reinterpret_cast<const char*>(&file.common.bit_field), sizeof(uint8_t));
		os.write(reinterpret_cast<const char*>(&file.common.alpha_chnl), sizeof(uint8_t));
		os.write(reinterpret_cast<const char*>(&file.common.red_chnl), sizeof(uint8_t));
		os.write(reinterpret_cast<const char*>(&file.common.green_chnl), sizeof(uint8_t));
		os.write(reinterpret_cast<const char*>(&file.common.blue_chnl), sizeof(uint8_t));

		// PAGES 3
		block_type = 3;
		os.write(reinterpret_cast<const char*>(&block_type), sizeof(uint8_t));
		block_size = file.pages.size() * (file.pages[0].length() + 1);
		os.write(reinterpret_cast<const char*>(&block_size), sizeof(uint32_t));
		for (const auto& p : file.pages)
			os.write(p.c_str(), static_cast<int>(p.length() + 1));

		// CHARS 4
		block_type = 4;
		os.write(reinterpret_cast<const char*>(&block_type), sizeof(uint8_t));
		block_size = file.chars.size() * sizeof(BMFChar);
		os.write(reinterpret_cast<const char*>(&block_size), sizeof(uint32_t));
		for (const auto c : file.chars)
			os.write(reinterpret_cast<const char*>(&c), sizeof(BMFChar));

		// KERNING PAIRS 5
	}

	void save_font(const std::string& font_file, const BMFFile& file)
	{
		log->info("Saving font: {}", font_file);
		std::fstream fs(font_file, std::fstream::out | std::fstream::binary);
		if (!fs)
			throw std::runtime_error("Could not open file: " + font_file);

		save_bmf_file(file, fs);
	}

	void create_default_font(const std::string& font_file)
	{
		BMFFile file;

		// HEADER
		file.header.id[0] = 'B';
		file.header.id[1] = 'M';
		file.header.id[2] = 'F';
		file.header.version = 3;

		// INFO
		memset(&file.info, 0, sizeof(BMFInfo));
		file.info.font_size = 16;
		file.info.outline = 1;
		file.info.stretch_h = 100;
		file.info.bit_field = 1; // unicode
		file.font_name = "default";

		// COMMON
		memset(&file.common, 0, sizeof(BMFCommon));
		file.common.line_height = 16;
		file.common.base = 12;
		file.common.scale_w = 256;
		file.common.scale_h = 256;
		file.common.pages = 1;

		// PAGES
		file.pages.push_back("default_0.png");

		// CHARS
		for (auto i = 32; i < 128; i++)
		{
			BMFChar c;
			c.id = i;
			c.x = ((i - 32) % 16) * 16;
			c.y = ((i - 32) / 16) * 16;
			c.width = 16;
			c.height = 16;
			c.x_offset = 0;
			c.y_offset = 0;
			c.x_advance = 16;
			c.page = 0;
			c.channel = 15;
			file.chars.push_back(c);
		}

		save_font(font_file, file);
	}
}