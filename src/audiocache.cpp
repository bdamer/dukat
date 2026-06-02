#include "stdafx.h"
#include <dukat/audiocache.h>
#include <dukat/log.h>
#include <dukat/string.h>
#include <dukat/sysutil.h>

namespace dukat
{
	AudioCache::~AudioCache(void)
	{
		if (preload_thread.joinable())
			preload_thread.join();
		free_all();
	}

	void AudioCache::preload_samples(void)
	{
		log->info("Preloading audio samples from: {}", sample_dir);
		preload_thread = std::thread([this]() {
			const auto start = SDL_GetTicks();
			const auto files = list_files(sample_dir);
			for (const auto& f : files)
			{
				const auto ext = file_extension(f);
				if (ext == "mp3" || ext == "wav" || ext == "ogg")
				{
					const auto id = compute_hash(f);
					auto sample = load_sample(f);
					std::lock_guard<std::mutex> lock(sample_mutex);
					if (!sample_map.count(id))
						sample_map[id] = std::move(sample);
				}
			}
			log->debug("Preload complete in: {}", SDL_GetTicks() - start);
		});
	}

	std::unique_ptr<Sample> AudioCache::load_sample(const std::string& filename)
	{
		log->trace("Loading sample [{}]", filename);
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
		{
			std::lock_guard<std::mutex> lock(sample_mutex);
			auto it = sample_map.find(id);
			if (it != sample_map.end())
				return it->second.get();
		}
		auto sample = load_sample(filename);
		std::lock_guard<std::mutex> lock(sample_mutex);
		auto& slot = sample_map[id];
		if (!slot)
			slot = std::move(sample);
		return slot.get();
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
		std::lock_guard<std::mutex> lock(sample_mutex);
		sample_map.erase(id);
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