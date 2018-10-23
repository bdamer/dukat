#pragma once

namespace dukat
{
	struct Texture;

    extern std::unique_ptr<Texture> generate_noise_texture(int width, int height);
}