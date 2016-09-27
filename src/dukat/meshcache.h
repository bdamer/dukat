#pragma once

#include <map>
#include <memory>

namespace dukat
{
	class Mesh;

	class MeshCache
	{
	private:
		std::map<std::string, std::unique_ptr<Mesh>> cache;

	public:
		Mesh* put(const std::string& id, std::unique_ptr<Mesh> mesh);
		Mesh* get(const std::string& id) const;
		void remove(const std::string& id) { cache.erase(id); }
		bool contains(const std::string& id) const { return cache.count(id) > 0; }
	};
}