#include "stdafx.h"
#include <dukat/audiomanager.h>
#include <dukat/audiocache.h>
#include <dukat/sdlutil.h>
#include <dukat/log.h>

namespace dukat
{
	AudioManager::AudioManager(int num_channels) : num_channels(num_channels), channel_volume(num_channels)
	{
		log->info("Initializing audio mixer.");

		const auto init_flags = MIX_INIT_MP3 | MIX_INIT_OGG;
		auto res = Mix_Init(init_flags);
		log->debug("Mix_Init returned: {}", res);
		if (res != init_flags)
			throw std::runtime_error("Failed to initialize audio mixer.");

		const auto frequency = 44100;
		sdl_check_result(Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, 2, 4096), "Open mixer");
		res = Mix_AllocateChannels(num_channels);
		log->debug("Allocated {} channels.", res);
		std::fill(channel_volume.begin(), channel_volume.end(), 1.0f);
	}

	AudioManager::~AudioManager(void)
	{
		Mix_CloseAudio();
		Mix_Quit();
	}

	int AudioManager::get_num_active_channels(void) const
	{
		return Mix_Playing(-1);
	}

	bool AudioManager::is_active(int channel) const
	{
		assert(channel < num_channels);
		return Mix_Playing(channel) != 0;
	}

	void AudioManager::set_music_volume(float volume)
	{
		music_volume = volume;
		if (Mix_PlayingMusic())
		{
			Mix_VolumeMusic(static_cast<int>(music_volume * MIX_MAX_VOLUME));
		}
	}

	void AudioManager::play_music(Music* music, int loops) const
	{
		if (Mix_PlayMusic(music->music, loops) == playback_error)
		{
			log->warn("Failed to play music: {}", Mix_GetError());
		}
		Mix_VolumeMusic(static_cast<int>(music_volume * MIX_MAX_VOLUME));
	}

	void AudioManager::pause_music(void) const
	{
		Mix_PauseMusic();
	}

	void AudioManager::stop_music(void) const
	{
		Mix_HaltMusic();
	}

	int AudioManager::play_sample(Sample* sample, int channel, int loops)
	{
		const auto res = Mix_PlayChannel(channel, sample->chunk, loops);
		if (res != playback_error)
		{
			Mix_Volume(res, static_cast<int>(channel_volume[res] * sample_volume * MIX_MAX_VOLUME));
		}
		else
		{
			// Playback failed - no channel available
		}
		return res;
	}
	
	void AudioManager::stop_sample(int channel)
	{
		Mix_HaltChannel(channel);
	}

	void AudioManager::stop_all(void)
	{
		for (auto channel = 0; channel < num_channels; channel++)
			Mix_HaltChannel(channel);
	}

	void AudioManager::pause_all(void)
	{
		Mix_Pause(-1);
		Mix_PausedMusic();
	}

	void AudioManager::resume_all(void)
	{
		Mix_Resume(-1);
		Mix_ResumeMusic();
	}
}