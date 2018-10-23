#include "stdafx.h"
#include <dukat/meshcache.h>
#include <dukat/meshdata.h>
#include <dukat/buffers.h>
#include <dukat/log.h>

namespace dukat
{
	MeshData* MeshCache::put(const std::string& id, std::unique_ptr<MeshData> get_mesh)
	{
		if (cache.count(id) > 0)
		{
			log->debug("Overriding mesh cache entry: {}", id);
		}
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