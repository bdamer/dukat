#include "stdafx.h"
#include "audiocache.h"
#include "log.h"
#include "sysutil.h"

namespace dukat
{
	std::unique_ptr<Sample> AudioCache::load_sample(const std::string& filename)
	{
		logger << "Loading sample [" << filename << "]: " << std::endl;
		auto fqn = sample_dir + "/" + filename;
		auto chunk = Mix_LoadWAV(fqn.c_str());
		if (chunk == nullptr)
		{
			logger << "Warning: " << Mix_GetError() << std::endl;
			std::stringstream ss;
			ss << "Could not load sample: " << filename;
			throw std::runtime_error(ss.str());
		}
		return std::make_unique<Sample>(chunk);
	}

	std::unique_ptr<Music> AudioCache::load_music(const std::string& filename)
	{
		logger << "Loading music [" << filename << "]: " << std::endl;
		auto fqn = music_dir + "/" + filename;
		auto music = Mix_LoadMUS(fqn.c_str());
		if (music == nullptr)
		{
			logger << "Warning: " << Mix_GetError() << std::endl;
			std::stringstream ss;
			ss << "Could not load music: " << filename;
			throw std::runtime_error(ss.str());
		}
		return std::make_unique<Music>(music);
	}

	Sample* AudioCache::get_sample(const std::string& filename)
	{
		const auto id = compute_hash(filename);
		if (sample_map.count(id) == 0)
		{
			sample_map[id] = load_sample(filename);
		}
		return sample_map[id].get();
	}

	Music* AudioCache::get_music(const std::string& filename)
	{
		const auto id = compute_hash(filename);
		if (music_map.count(id) == 0)
		{
			music_map[id] = load_music(filename);
		}
		return music_map[id].get();
	}
	
	void AudioCache::free_all(void)
	{
		logger << "Freeing " << sample_map.size() << " sample(s)..." << std::endl;
		sample_map.clear();
		logger << "Freeing " << music_map.size() << " track(s)..." << std::endl;
		music_map.clear();
	}

	void AudioCache::free_sample(const std::string& filename)
	{
		auto id = compute_hash(filename);
		if (sample_map.count(id))
		{
			sample_map.erase(id);
		}
	}

	void AudioCache::free_music(const std::string& filename)
	{
		auto id = compute_hash(filename);
		if (music_map.count(id))
		{
			music_map.erase(id);
		}
	}
}