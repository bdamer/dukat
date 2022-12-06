#pragma once

#include <map>
#include <string>

namespace dukat
{
	class Settings
	{
	private:
		std::map<std::string, std::string> map;
		void load_ini(const std::string& filename);

	public:
		Settings(void) { }
		Settings(const std::string& filename);
		Settings(const Settings& settings);
		~Settings(void) { }

		// Getters
		std::string get_string(const std::string& name, const std::string& default_value = "") const;
		int get_int(const std::string& name, int default_value = 0) const;
		float get_float(const std::string& name, float default_value = 0.0f) const;
		bool get_bool(const std::string& name, bool default_value = false) const;

		// Setters
		void set(const std::string& name, const std::string& value);
		void set(const std::string& name, int value);
		void set(const std::string& name, float value);
		void set(const std::string& name, bool value);

		friend void save_settings(const Settings& settings, const std::string& filename);
		friend void load_settings(const std::string& filename, Settings& settings);
	};

	// Attempts to load JSON settings based on prefix from path. If not found, will load INI settings instead.
	Settings load_settings_with_default(const std::string& path, const std::string& prefix);
}