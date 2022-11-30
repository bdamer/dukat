#pragma once

#include <dukat/color.h>

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

	inline constexpr SDL_Color sdl_color(const Color& color)
	{
		return SDL_Color{
			static_cast<uint8_t>(color.r * 255.f),
			static_cast<uint8_t>(color.g * 255.f),
			static_cast<uint8_t>(color.b * 255.f),
			static_cast<uint8_t>(color.a * 255.f)
		};
	}

	// Display mode utilties
	std::string format_display_mode(const SDL_DisplayMode& mode);
	std::vector<std::string> list_display_names(void);
	std::vector<SDL_DisplayMode> list_display_modes(int display_idx);
	int find_current_display_mode(int display_idx, const std::vector<SDL_DisplayMode>& modes);
}