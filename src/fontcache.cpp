#include "stdafx.h"
#include <dukat/fontcache.h>

namespace dukat
{
	BitmapFont* FontCache::get(const std::string& name)
	{
		if (fonts.count(name) == 0)
			fonts[name] = load_font(resource_dir + "/" + name, filter_profile);
		return fonts.at(name).get();
	}
}