#include "stdafx.h"
#include <dukat/string.h>

namespace dukat
{
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

	uint32_t compute_hash(const std::string& s)
	{
		// pad size to multiple of 4
		int size = (s.length() + 3) & ~0x03;
		std::vector<unsigned char> buffer(size);
		std::transform(s.begin(), s.end(), buffer.begin(), toupper);

		// process characters in groups of 4 bytes
		uint32_t res = 0;
		for (int i = 0; i < size; i += 4)
		{
			res = (res << 1) | (res >> 31); // rotate left by 1
			res += (buffer[i + 3] << 24) + (buffer[i + 2] << 16) + (buffer[i + 1] << 8) + buffer[i]; // accumulate
		}
		return res;
	}
}