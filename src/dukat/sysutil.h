#pragma once

namespace dukat
{
	// Checks a SDL result for errors. If there is an error, will 
	// throw a runtime exception.
	void sdl_check_result(int res, const std::string& operation);

	uint32_t swape32(uint32_t value);

	uint16_t swape16(uint16_t value);

	inline std::string get_extension(const std::string& filename)
	{
		return filename.substr(filename.rfind('.') + 1);
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

	// reads a pascal string from a stream.
	std::string read_pstring(std::istream& is);
	// writes a pascal string to a stream.
	size_t write_pstring(std::ostream& os, const std::string& str);
}