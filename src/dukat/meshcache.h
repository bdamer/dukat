#pragma once

#include <map>
#include <memory>

namespace dukat
{
	class Mesh;

	// TODO: how do things get evicted from this cache?
	class MeshCache
	{
	private:
		int last_id;
		std::map<std::string, std::unique_ptr<Mesh>> cache;

	public:
		MeshCache(void) : last_id(0) { }
		~MeshCache(void) { }

		Mesh* put(const std::string& id, std::unique_ptr<Mesh> mesh);
		Mesh* get(const std::string& id) const;
		void remove(const std::string& id) { cache.erase(id); }
		bool contains(const std::string& id) const { return cache.count(id) > 0; }
		std::string generate_id(void);
	};
}