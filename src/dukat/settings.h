#pragma once

#include <map>
#include <string>

namespace dukat
{
	class Settings
	{
	private:
		std::map<std::string, std::string> map;

	public:
		Settings(const std::string& filename);
		~Settings(void) { }

		std::string get_string(const std::string& name, const std::string& default_value = "") const;
		int get_int(const std::string& name, int default_value = 0) const;
		float get_float(const std::string& name, float default_value = 0.0f) const;
		bool get_bool(const std::string& name, bool default_value = false) const;
	};
}