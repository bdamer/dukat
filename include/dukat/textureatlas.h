#pragma once

#include "rect.h"

namespace dukat
{
	class TextureAtlas
	{
	private:
		std::unordered_map<std::string,Rect> entries;

	public:
		TextureAtlas(void) { }
		~TextureAtlas(void) { }

		void add(const std::string& name, const Rect& rect) { entries[name] = rect; }
		void remove(const std::string& name) { entries.erase(name); }
		Rect get(const std::string& name) const;
		bool contains(const std::string& name) const { return entries.count(name) > 0; }
		size_t size(void) const { return entries.size(); }

		// Stream input / output
		friend std::ostream& operator<<(std::ostream& os, const TextureAtlas& v);
		friend std::istream& operator>>(std::istream& is, TextureAtlas& v);
	};
}