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

	inline bool ends_with(const std::string& haystack, const std::string& needle)
	{
		return needle.length() <= haystack.length() && equal(needle.begin(), needle.end(), haystack.end() - needle.size());
	}

	inline std::string join(const std::vector<std::string>& v, const char* delim = ",")
	{
		std::ostringstream oss;
		if (!v.empty())
		{
			for (auto i = 0u; i < v.size() - 1; i++)
				oss << v[i] << delim;
			oss << *(v.end() - 1);
		}
		return oss.str();
	}

	template<typename Iterator>
	std::string join(Iterator first, Iterator last, const char* delim = ",")
	{
		std::ostringstream oss;
		if (first != last)
		{
			std::copy(first, last - 1, std::ostream_iterator<int>(oss, delim));
			oss << std::to_string(*(last - 1));
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

	// Returns a version of the input string with line breaks added
	// so that no line is longer than num_cols.
	std::string add_line_breaks(const std::string& input, int num_cols);
}