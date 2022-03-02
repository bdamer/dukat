#pragma once

namespace dukat
{
	// Checks a SDL result for errors. If there is an error, will 
	// throw a runtime exception.
	void sdl_check_result(int res, const std::string& operation);

	inline SDL_Color sdl_color_rgb(uint32_t rgb)
	{
		SDL_Color c;
		c.r = static_cast<uint8_t>((rgb >> 16) & 0xff);
		c.g = static_cast<uint8_t>((rgb >> 8) & 0xff);
		c.b = static_cast<uint8_t>((rgb) & 0xff);
		c.a = static_cast<uint8_t>(0xff);
		return c;
	}

	inline SDL_Color sdl_color_rgba(uint32_t rgba)
	{
		SDL_Color c;
		c.r = static_cast<uint8_t>((rgba >> 24) & 0xff);
		c.g = static_cast<uint8_t>((rgba >> 16) & 0xff);
		c.b = static_cast<uint8_t>((rgba >> 8) & 0xff);
		c.a = static_cast<uint8_t>((rgba) & 0xff);
		return c;
	}
}