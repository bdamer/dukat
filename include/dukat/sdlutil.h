#pragma once

namespace dukat
{
	// Checks a SDL result for errors. If there is an error, will 
	// throw a runtime exception.
	void sdl_check_result(int res, const std::string& operation);

	inline SDL_Color sdl_color_rgb(uint32_t rgb)
	{
		return SDL_Color{ (rgb >> 16) & 0xff, (rgb >> 8) & 0xff, (rgb) & 0xff, 0xff };
	}

	inline SDL_Color sdl_color_rgba(uint32_t rgba)
	{
		return SDL_Color{ (rgba >> 24) & 0xff, (rgba >> 16) & 0xff, (rgba >> 8) & 0xff, (rgba) & 0xff };
	}
}