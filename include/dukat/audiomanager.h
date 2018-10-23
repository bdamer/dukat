#pragma once

namespace dukat
{
	struct Music;
	struct Sample;

	class AudioManager
	{
	private:
		int num_channels;
		float music_volume;
		float sample_volume;
		std::vector<float> channel_volume;

	public:
		// Creates an audio manager with a specified number of mixer channels.
		AudioManager(int num_channels);
		~AudioManager(void);

		int get_num_channels(void) const { return num_channels; }
		float get_music_volume(void) const { return music_volume; }
		// Note: music volume for midi files changes global application volume.
		void set_music_volume(float volume);
		float get_sample_volume(void) const { return sample_volume; }
		void set_sample_volume(float volume) { sample_volume = volume; }
		float get_channel_volume(int channel) const { return channel_volume[channel]; }
		void set_channel_volume(int channel, float volume) { channel_volume[channel] = volume; }

		// Plays music. Number of loops can be set to -1 to repeat.
		void play_music(Music* music, int loops = 1) const;
		void pause_music(void) const;
		void stop_music(void) const;

		// Plays a sample. If no channel specified, will use next available channel. Will
		// return the channel user or -1 in case of error.
		int play_sample(Sample* sample, int channel = -1, int loops = 0);
		void stop_sample(int channel);
	};
}