#include "stdafx.h"
#include <dukat/settings.h>
#include <dukat/log.h>
#include <dukat/assetloader.h>

namespace dukat
{
	Settings::Settings(const std::string& filename)
	{
		log->info("Loading settings from: {}", filename);
		load_ini(filename);
	}

	Settings::Settings(const Settings& settings)
	{
		for (auto it = settings.map.begin(); it != settings.map.end(); ++it)
		{
			map[(*it).first] = (*it).second;
		}
	}

	void Settings::load_ini(const std::string& filename)
	{
		std::stringstream ss;
		AssetLoader loader;
		loader.load_text(filename, ss);
		std::string line;
		while (std::getline(ss, line))
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

	void Settings::set(const std::string& name, const std::string& value)
	{
		map[name] = value;
	}

	void Settings::set(const std::string& name, int value)
	{
		map[name] = std::to_string(value);
	}

	void Settings::set(const std::string& name, float value)
	{
		map[name] = std::to_string(value);
	}

	void Settings::set(const std::string& name, bool value)
	{
		map[name] = value ? "true" : "false";
	}
	
	Settings load_settings_with_default(const std::string& path, const std::string& prefix)
	{
		// TODO: load all files via asset loader
		AssetLoader loader;
		const auto file = path + "/" + prefix + ".json";
		if (loader.exists(file))
		{
			Settings settings;
			load_settings(file, settings);
			return settings;
		}
		else
		{
			return Settings(path + "/" + prefix + ".ini");
		}
	}
}