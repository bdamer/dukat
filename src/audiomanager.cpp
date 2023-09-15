#include "stdafx.h"
#include <dukat/audiomanager.h>
#include <dukat/audiocache.h>
#include <dukat/sdlutil.h>
#include <dukat/log.h>

namespace dukat
{
	AudioManager::AudioManager(int num_channels) : initialized(false), num_channels(num_channels), channel_volume(num_channels)
	{
		log->info("Initializing audio mixer.");

		const auto init_flags = MIX_INIT_MP3 | MIX_INIT_OGG;
		auto res = Mix_Init(init_flags);
		log->debug("Mix_Init returned: {}", res);
		if (res != init_flags)
			throw std::runtime_error("Failed to initialize audio mixer.");

		const auto frequency = 44100;

		// This can fail if no audio devices are available.
		res = Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, 2, 4096);
		if (res < 0)
		{
			log->warn("Failed to open audio device: {}", SDL_GetError());
			return; 
		}

		res = Mix_AllocateChannels(num_channels);
		log->debug("Allocated {} channels.", res);
		std::fill(channel_volume.begin(), channel_volume.end(), 1.0f);

		initialized = true;
	}

	AudioManager::~AudioManager(void)
	{
		if (initialized)
		{
			Mix_CloseAudio();
			Mix_Quit();
		}
	}

	int AudioManager::get_num_active_channels(void) const
	{
		return initialized ? Mix_Playing(-1) : 0;
	}

	bool AudioManager::is_active(int channel) const
	{
		assert(channel < num_channels);
		return initialized ? Mix_Playing(channel) != 0 : false;
	}

	void AudioManager::set_music_volume(float volume)
	{
		music_volume = volume;
		if (initialized && Mix_PlayingMusic())
			Mix_VolumeMusic(static_cast<int>(music_volume * MIX_MAX_VOLUME));
	}

	void AudioManager::play_music(Music* music, int loops) const
	{
		if (!initialized) return;
		if (Mix_PlayMusic(music->music, loops) == playback_error)
			log->warn("Failed to play music: {}", Mix_GetError());
		Mix_VolumeMusic(static_cast<int>(music_volume * MIX_MAX_VOLUME));
	}

	void AudioManager::pause_music(void) const
	{
		if (!initialized) return;
		Mix_PauseMusic();
	}

	void AudioManager::stop_music(void) const
	{
		if (!initialized) return;
		Mix_HaltMusic();
	}

	bool AudioManager::is_playing_music(void) const
	{
		return initialized && Mix_PlayingMusic() > 0;
	}

	int AudioManager::fade_in_music(float duration, Music* music, int loops) const
	{
		return initialized && Mix_FadeInMusic(music->music, loops, static_cast<int>(duration * 1000.f));
	}

	int AudioManager::fade_out_music(float duration) const
	{
		return initialized && Mix_FadeOutMusic(static_cast<int>(duration * 1000.f));
	}

	int AudioManager::play_sample(Sample* sample, int channel, int loops)
	{
		if (!initialized) return playback_error;
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
		if (!initialized) return;
		Mix_HaltChannel(channel);
	}

	void AudioManager::stop_all(void)
	{
		if (!initialized) return;
		for (auto channel = 0; channel < num_channels; channel++)
			Mix_HaltChannel(channel);
	}

	void AudioManager::pause_all(void)
	{
		if (!initialized) return;
		Mix_Pause(-1);
		Mix_PauseMusic();
	}

	void AudioManager::resume_all(void)
	{
		if (!initialized) return;
		Mix_Resume(-1);
		Mix_ResumeMusic();
	}
}