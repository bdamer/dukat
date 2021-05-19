#include "stdafx.h"
#include <dukat/json.h>
#include <dukat/log.h>
#include <json/json.h>
#include <dukat/settings.h>

namespace dukat
{
	Json::Value load_json(const std::string& filename)
	{
		Json::Value root;
		load_json(filename, root);
		return root;
	}

	void load_json(const std::string& filename, Json::Value& root)
	{
		Json::Reader reader;
		log->debug("Loading: {}", filename);
		std::fstream fs(filename, std::fstream::in);
		if (!reader.parse(fs, root))
		{
			log->warn("Failed to parse JSON: {}", reader.getFormattedErrorMessages());
			throw std::runtime_error("Failed to parse JSON.");
		}
	}

	void merge_json_subtree(Json::Value& base, const Json::Value& patch)
	{
		for (auto p : patch.getMemberNames())
		{
			auto& dest = base[p];
			const auto& src = patch[p];
			if (src.isObject() && dest.isObject())
			{
				merge_json_subtree(dest, src);
			}
			else if (src.isArray() && dest.isArray())
			{
				assert(src.size() == dest.size());
				for (auto i = 0u; i < src.size(); i++)
				{
					if (src[i].isObject())
						merge_json_subtree(dest[i], src[i]);
					else
						dest[i] = src[i];
				}
			}
			else // primitive
			{
				dest = src;
			}
		}
	}

	Json::Value merge_json(Json::Value base, const Json::Value& patch)
	{
		merge_json_subtree(base, patch);
		return base;
	}

	void save_json(const std::string& filename, const Json::Value& root)
	{
		log->debug("Saving: {}", filename);
		std::fstream fs(filename, std::fstream::out);
		if (!fs)
			throw std::runtime_error("Failed to write JSON.");

		Json::StyledStreamWriter writer;
		writer.write(fs, root);
		fs.close();
	}

	void save_settings(const Settings& settings, const std::string& filename)
	{
		log->info("Saving settings to: {}", filename);
		Json::Value root;
		for (const auto& it : settings.map)
		{
			Json::Value* cur = &root;
			auto from = 0u;
			auto to = it.first.find('.');
			while (to != std::string::npos)
			{
				cur = &(*cur)[it.first.substr(from, to - from)];
				from = to + 1;
				to = it.first.find('.', from);
			}
			(*cur)[it.first.substr(from)] = it.second;
		}
		save_json(filename, root);
	}

	void load_settings_from_json(const Json::Value& node, const std::string prefix, std::map<std::string, std::string>& map)
	{
		if (node.isObject())
		{
			for (const auto& key : node.getMemberNames())
				load_settings_from_json(node[key], (prefix.length() ? prefix + "." : "") + key, map);
		}
		else if (node.isArray())
		{
			auto i = 0;
			for (const auto& val : node)
				load_settings_from_json(val, (prefix.length() ? prefix + "." : "") + std::to_string(i++), map);
		}
		else
		{
			map[prefix] = node.asString();
		}
	}

	void load_settings(const std::string& filename, Settings& settings)
	{
		log->info("Loading settings from: {}", filename);
		const auto root = load_json(filename);
		load_settings_from_json(root, "", settings.map);
	}
}