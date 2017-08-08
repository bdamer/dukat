#include "stdafx.h"
#include "textureutil.h"
#include "texture.h"

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
		auto res = std::make_unique<Texture>(width, height, ProfileNearest);
		glBindTexture(GL_TEXTURE_2D, res->id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, data.data());
        return std::move(res);
    }
}