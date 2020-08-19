#include "stdafx.h"
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
}