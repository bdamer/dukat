#include "stdafx.h"
#include "meshcache.h"
#include "meshdata.h"
#include "buffers.h"

namespace dukat
{
	MeshData* MeshCache::put(const std::string& id, std::unique_ptr<MeshData> get_mesh)
	{
		cache[id] = std::move(get_mesh);
		return cache[id].get();
	}
	
	MeshData* MeshCache::get(const std::string& id) const
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

	std::string MeshCache::generate_id(void)
	{
		std::stringstream ss;
		ss << "_generated_" << ++last_id;
		return ss.str();
	}
}