#include "stdafx.h"
#include <dukat/textureutil.h>
#include <dukat/texture.h>
#include <dukat/sysutil.h>

namespace dukat
{
    std::unique_ptr<Texture> generate_noise_texture(int width, int height)
    {
        const auto texture_size = width * height;
        std::vector<uint32_t> data(texture_size);
        // Generate texture with noise values in RG components
        for (int i = 0; i < texture_size; i++)
        {
            data[i] = ( (static_cast<uint8_t>(std::rand() % 256) << 24) | 
                        (static_cast<uint8_t>(std::rand() % 256) << 16) | 
                        (0xff << 8) |
                        0xff );
        }
		// Upload to texture
		TextureBuilder tb;
		auto res = tb.set_width(width).set_height(height)
			.set_filter_profile(ProfileNearest)
			.set_internal_format(GL_RGBA8)
			.set_format(GL_RGBA)
#ifdef OPENGL_CORE
			.set_type(GL_UNSIGNED_INT_8_8_8_8)
#else
			.set_type(GL_UNSIGNED_BYTE)
#endif
			.build(data.data());
        return res;
    }

	TextureBuilder::TextureBuilder(void)
	{
		filter_profile = TextureFilterProfile::ProfileLinear;
		target = GL_TEXTURE_2D;
		wrap = GL_REPEAT;
		internal_format = 0;
		format = 0;
		type = 0;
		width = 0;
		height = 0;
		depth = 1;
	}

	std::unique_ptr<Texture> TextureBuilder::build(void* data)
	{
		auto res = std::make_unique<Texture>(width, height, filter_profile);
		glBindTexture(target, res->id);

		switch (filter_profile)
		{
		case ProfileNearest:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;

		case ProfileLinear:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;

		case ProfileMipMapped:
		case ProfileAnisotropic:
		default:
			break;
		}

		glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);
		glTexImage2D(target, 0, internal_format, width, height, 0, format, type, data);

		gl_check_error();

		return std::move(res);
	}
}