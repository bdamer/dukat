#include "stdafx.h"
#include <dukat/textureatlas.h>

namespace dukat
{
	std::ostream& operator<<(std::ostream& os, const TextureAtlas& v)
	{
		for (const auto& e : v.entries)
			os << e.first << " " << e.second.x << " " << e.second.y << " " << e.second.w << " " << e.second.h << std::endl;
		return os;
	}

	std::istream& operator>>(std::istream& is, TextureAtlas& v)
	{
		std::string name;
		Rect r;
		while (!is.eof())
		{
			is >> name >> r.x >> r.y >> r.w >> r.h;
			v.add(name, r);
		}
		return is;
	}
	
	Rect TextureAtlas::get(const std::string& name) const
	{
		if (!contains(name))
			throw std::runtime_error("Invalid texture atlas entry: " + name);
		return entries.at(name);
	}
}