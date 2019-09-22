#pragma once

#include <json/json-forwards.h>

namespace dukat
{
	// JSON helpers
	Json::Value load_json(const std::string& filename);
	Json::Value merge_json(Json::Value base, const Json::Value& patch);
	void save_json(const std::string& filename, const Json::Value& root);
}