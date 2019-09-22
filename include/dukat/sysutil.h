#pragma once

namespace dukat
{
	// Checks a SDL result for errors. If there is an error, will 
	// throw a runtime exception.
	void sdl_check_result(int res, const std::string& operation);

	void gl_check_error(void);

	inline constexpr uint32_t mc_const(char a, char b, char c, char d) 
	{
		return static_cast<uint32_t>((a << 24) | (b << 16) | (c << 8) | d);
	}

	inline uint16_t read_short(const uint8_t* ptr)
	{
		return (*(ptr + 1) << 8) | (*ptr);
	}

	inline uint32_t read_int(const uint8_t* ptr)
	{
		return (*(ptr + 3) << 24) | (*(ptr + 2) << 16) | (*(ptr + 1) << 8) | (*ptr);
	}

	inline float_t read_float(const uint8_t* ptr)
	{
		auto val = read_int(ptr);
		return reinterpret_cast<float_t&>(val);
	}

	// Writes the content of the current screen buffer to a file.
	void save_screenshot(const std::string& filename);

}