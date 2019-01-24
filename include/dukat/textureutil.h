#pragma once

#include "texture.h"

namespace dukat
{
	class TextureBuilder
	{
	private:
		TextureFilterProfile filter_profile;
		GLenum target;
		GLenum wrap;
		GLint internal_format;
		GLenum format;
		GLenum type;
		int width;
		int height;
		int depth;

	public:
		TextureBuilder(void);
		~TextureBuilder(void) { }

		TextureBuilder& set_filter_profile(TextureFilterProfile profile) { this->filter_profile = profile; return *this; }
		TextureBuilder& set_target(GLenum target) { this->target = target; return *this; }
		TextureBuilder& set_wrap(GLenum wrap) { this->wrap = wrap; return *this; }
		TextureBuilder& set_internal_format(GLint internal_format) { this->internal_format = internal_format; return *this; }
		TextureBuilder& set_format(GLenum format) { this->format = format; return *this; }
		TextureBuilder& set_type(GLenum type) { this->type = type; return *this; }
		TextureBuilder& set_width(int width) { this->width = width; return *this; }
		TextureBuilder& set_height(int height) { this->height = height; return *this; }
		TextureBuilder& set_depth(int depth) { this->depth = depth; return *this; }

		std::unique_ptr<Texture> build(void* data = nullptr);
	};

	extern std::unique_ptr<Texture> generate_noise_texture(int width, int height);
}