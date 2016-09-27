#include "stdafx.h"
#include "meshcache.h"
#include "mesh.h"
#include "buffers.h"

namespace dukat
{
	Mesh* MeshCache::put(const std::string& id, std::unique_ptr<Mesh> get_mesh)
	{
		cache[id] = std::move(get_mesh);
		return cache[id].get();
	}
	
	Mesh* MeshCache::get(const std::string& id) const
	{
		if (cache.count(id))
		{
			return cache.at(id).get();
		}
		else
		{
			return nullptr;
		}
	}
}