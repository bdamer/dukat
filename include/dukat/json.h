#pragma once

#include <json/json-forwards.h>
#include <dukat/settings.h>

namespace dukat
{
	// JSON helpers
	Json::Value load_json(const std::string& filename);
	void load_json(const std::string& filename, Json::Value& root);
	Json::Value merge_json(Json::Value base, const Json::Value& patch);
	void save_json(const std::string& filename, const Json::Value& root);

	// Attempts to load JSON settings based on prefix from path. If not found, will load INI settings instead.
	Settings load_settings_with_default(const std::string& path, const std::string& prefix);
}