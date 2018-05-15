#include "stdafx.h"
#include "audiomanager.h"
#include "audiocache.h"
#include "sysutil.h"
#include "log.h"

namespace dukat
{
	AudioManager::AudioManager(int num_channels) : num_channels(num_channels), channel_volume(num_channels)
	{
		logger << "Initializating audio mixer..." << std::endl;
		sdl_check_result(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096), "Open mixer");
		auto res = Mix_AllocateChannels(num_channels);
		logger << "Allocated " << res << " channels." << std::endl;
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
			logger << "Failed to play music: " << Mix_GetError() << std::endl;
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

	void AudioManager::play_sample(Sample* sample, int channel, int loops)
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
	}

	void AudioManager::stop_sample(int channel)
	{
		Mix_HaltChannel(channel);
	}
}