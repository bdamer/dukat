#include "stdafx.h"
#include "settings.h"
#include "log.h"

namespace dukat
{
	Settings::Settings(const std::string& filename)
	{
		logger << "Loading settings from: " << filename << std::endl;
		std::fstream is(filename);
		if (!is)
		{
			throw std::runtime_error("Could not load file: " + filename);
		}
		std::string line;
		while (std::getline(is, line))
		{
			if (line.size() == 0 || line[0] == ';')
				continue;

			std::stringstream ss(line);
			auto idx = line.find("=");
			if (idx >= 0)
			{
				auto key = line.substr(0, idx);
				auto val = line.substr(idx + 1);
				map[key] = val;
			}
		}
		is.close();
	}

	std::string Settings::get_string(const std::string& name, const std::string & default_value) const
	{
		return map.count(name) > 0 ? map.at(name) : default_value;
	}

	int Settings::get_int(const std::string& name, int default_value) const
	{
		return map.count(name) > 0 ? std::stoi(map.at(name)) : default_value;
	}

	float Settings::get_float(const std::string& name, float default_value) const
	{
		return map.count(name) > 0 ? std::stof(map.at(name)) : default_value;
	}

	bool Settings::get_bool(const std::string& name, bool default_value) const
	{
		return map.count(name) > 0 ? map.at(name) == "true" : default_value;
	}
}