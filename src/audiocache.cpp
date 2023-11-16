#include "stdafx.h"
#include <dukat/audiocache.h>
#include <dukat/log.h>
#include <dukat/string.h>
#include <dukat/sysutil.h>

namespace dukat
{
	void AudioCache::preload_samples(void)
	{
		log->info("Preloading audio samples from: {}", sample_dir);
		const auto start = SDL_GetTicks();
		const auto files = list_files(sample_dir);
		for (const auto& f : files)
		{
			const auto ext = file_extension(f);
			if (ext == "mp3" || ext == "wav" || ext == "ogg")
			{
				const auto id = compute_hash(f);
				sample_map[id] = load_sample(f);
			}
		}
		log->debug("Preload complete in: {}", SDL_GetTicks() - start);
	}

	std::unique_ptr<Sample> AudioCache::load_sample(const std::string& filename)
	{
		log->debug("Loading sample [{}]", filename);
		const auto start = SDL_GetTicks();
		auto fqn = sample_dir + "/" + filename;
		auto chunk = Mix_LoadWAV(fqn.c_str());
		if (chunk == nullptr)
		{
			log->warn("Failed to load {}: {}", filename, Mix_GetError());
			return nullptr;
		}
		log->trace("Load complete in: {}", SDL_GetTicks() - start);
		return std::make_unique<Sample>(chunk);
	}

	std::unique_ptr<Music> AudioCache::load_music(const std::string& filename)
	{
		log->debug("Loading music [{}]", filename);
		auto fqn = music_dir + "/" + filename;
		auto music = Mix_LoadMUS(fqn.c_str());
		if (music == nullptr)
		{
			log->warn("Failed to load {}: {}", filename, Mix_GetError());
			return nullptr;
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
		log->debug("Freeing {} sample(s)...", sample_map.size());
		sample_map.clear();
		log->debug("Freeing {} track(s)...", music_map.size());
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