#include "stdafx.h"
#include "sysutil.h"

namespace dukat
{
	void sdl_check_result(int res, const std::string& operation)
	{
		if (res < 0)
		{
			std::ostringstream ss;
			ss << "Failed to " << operation << ": " << SDL_GetError() << std::endl;
			throw std::runtime_error(ss.str());
		}
	}

	uint32_t swape32(uint32_t value)
	{
		uint32_t result = 0;
		result |= (value & 0x000000FF) << 24;
		result |= (value & 0x0000FF00) << 8;
		result |= (value & 0x00FF0000) >> 8;
		result |= (value & 0xFF000000) >> 24;
		return result;
	}

	uint16_t swape16(uint16_t value)
	{
		uint16_t result = 0;
		result |= (value & 0x00FF) << 8;
		result |= (value & 0xFF00) >> 8;
		return result;
	}

	std::string read_pstring(std::istream& is)
	{
		size_t size;
		is.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
		std::vector<char> buffer;
		buffer.resize(size);
		is.read(buffer.data(), size);
		return std::string(buffer.data());
	}

	size_t write_pstring(std::ostream& os, const std::string& str)
	{
		size_t size = str.length() + 1;
		os.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
		os.write(str.c_str(), size);
		return size + sizeof(uint32_t);
	}

}