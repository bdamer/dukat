#include "stdafx.h"
#include <dukat/json.h>
#include <dukat/log.h>
#include <json/json.h>

namespace dukat
{
	Json::Value load_json(const std::string& filename)
	{
		Json::Reader reader;
		Json::Value root;
		log->debug("Loading: {}", filename);
		std::fstream fs(filename, std::fstream::in);
		if (!reader.parse(fs, root))
		{
			log->warn("Failed to parse JSON: {}", reader.getFormattedErrorMessages());
			throw std::runtime_error("Failed to parse JSON.");
		}
		return root;
	}

	Json::Value merge_json(Json::Value base, const Json::Value & patch)
	{
		for (auto p : patch.getMemberNames())
		{
			// TODO: support patching partial trees
			base[p] = patch[p];
		}
		return base;
	}

	void save_json(const std::string& filename, const Json::Value& root)
	{
		std::fstream fs(filename, std::fstream::out);
		if (!fs)
			throw std::runtime_error("Failed to write JSON.");

		Json::StyledStreamWriter writer;
		writer.write(fs, root);
		fs.close();
	}
}