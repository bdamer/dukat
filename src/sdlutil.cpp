#include "stdafx.h"
#include <dukat/log.h>
#include <dukat/sdlutil.h>

namespace dukat
{
	void sdl_check_result(int res, const std::string& operation)
	{
		if (res < 0)
		{
			std::stringstream ss;
			ss << "Failed to " << operation << ": " << SDL_GetError();
			throw std::runtime_error(ss.str());
		}
	}

	std::string format_display_mode(const SDL_DisplayMode& mode, int flags)
	{
		std::stringstream ss;
		if ((flags & 1) == 1)
			ss << mode.w << "x" << mode.h << " ";
		if ((flags & 2) == 2)
			ss << "@ " << mode.refresh_rate << " Hz ";
		if ((flags & 4) == 4)
			ss << " " << SDL_GetPixelFormatName(mode.format) << "bit ";
		return ss.str();
	}

	std::vector<std::string> list_display_names(void)
	{
		const auto num_displays = SDL_GetNumVideoDisplays();
		std::vector<std::string> displays(num_displays);
		for (auto i = 0; i < num_displays; i++)
		{
			const auto name = SDL_GetDisplayName(i);
			if (name != nullptr)
			{
				log->debug("Found display {}: {}", i, name);
				displays[i] = std::string(name);
			}
		}
		return displays;
	}

	std::vector<SDL_DisplayMode> list_display_modes(int display_idx)
	{
		const auto num_modes = SDL_GetNumDisplayModes(display_idx);
		std::vector<SDL_DisplayMode> modes(num_modes);
		for (auto i = 0; i < num_modes; i++)
		{
			sdl_check_result(SDL_GetDisplayMode(display_idx, i, &modes[i]), "SDL_GetDisplayMode");
			log->debug("Found display mode {}: {}", i, format_display_mode(modes[i]));
		}
		return modes;
	}

	int find_current_display_mode(int display_idx, const std::vector<SDL_DisplayMode>& modes)
	{
		SDL_DisplayMode mode;
		sdl_check_result(SDL_GetCurrentDisplayMode(display_idx, &mode), "SDL_GetCurrentDisplayMode");
		for (auto i = 0; i < static_cast<int>(modes.size()); i++)
		{
			if (modes[i].w == mode.w && modes[i].h == mode.h && modes[i].refresh_rate == mode.refresh_rate && modes[i].format == mode.format)
				return i;
		}
		return -1;
	}
}