#pragma once

namespace dukat
{
	// Checks a SDL result for errors. If there is an error, will 
	// throw a runtime exception.
	void sdl_check_result(int res, const std::string& operation);
}