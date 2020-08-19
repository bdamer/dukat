#include "stdafx.h"
#include <dukat/audiomanager.h>
#include <dukat/audiocache.h>
#include <dukat/sdlutil.h>
#include <dukat/log.h>

namespace dukat
{
	AudioManager::AudioManager(int num_channels) : num_channels(num_channels), channel_volume(num_channels)
	{
		log->info("Initializating audio mixer.");
		sdl_check_result(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096), "Open mixer");
		auto res = Mix_AllocateChannels(num_channels);
		log->debug("Allocated {} channels.", res);
		for (auto& c : channel_volume)
			c = 1.0f;
	}

	AudioManager::~AudioManager(void)
	{
		Mix_CloseAudio();
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
		if (Mix_PlayMusic(music->music, loops) == -1)
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
		auto res = Mix_PlayChannel(channel, sample->chunk, loops);
		if (res > -1) 
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
}