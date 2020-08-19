#pragma once

#include <iterator>
#include <string>

namespace dukat
{
	// Begin inline string helpers

	inline bool starts_with(const std::string& haystack, const std::string& needle)
	{
		return needle.length() <= haystack.length() && equal(needle.begin(), needle.end(), haystack.begin());
	}

	template<typename T>
	std::string to_string(const std::vector<T>& vec)
	{
		std::ostringstream oss;
		if (!vec.empty())
		{
			std::copy(vec.begin(), vec.end() - 1, std::ostream_iterator<int>(oss, ","));
			oss << vec.back();
		}
		return oss.str();
	}

	// End inline string helpers

	// reads a pascal string from a stream.
	std::string read_pstring(std::istream& is);
	// writes a pascal string to a stream.
	size_t write_pstring(std::ostream& os, const std::string& str);

	// Computes hash value of provided string.
	uint32_t compute_hash(const std::string&);
}