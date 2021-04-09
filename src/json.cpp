#include "stdafx.h"
#include <dukat/json.h>
#include <dukat/log.h>
#include <json/json.h>

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
}