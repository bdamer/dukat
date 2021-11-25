#pragma once

#include <unordered_map>
#include <dukat/bitmapfont.h>

namespace dukat
{
	class FontCache
	{
	private:
		const std::string resource_dir;
		TextureFilterProfile filter_profile;
		std::unordered_map<std::string, std::unique_ptr<BitmapFont>> fonts;

	public:
		FontCache(const std::string& resource_dir) : resource_dir(resource_dir), filter_profile(TextureFilterProfile::ProfileNearest) { }
		~FontCache(void) { }

		void set_texture_filter_profile(TextureFilterProfile filter_profile) { this->filter_profile = filter_profile; }

		BitmapFont* get(const std::string& font);
		void clear(void) { fonts.clear(); }
	};
}