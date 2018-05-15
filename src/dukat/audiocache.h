#pragma once

#include <unordered_map>
#include <SDL2/SDL_mixer.h>

namespace dukat
{
	struct Sample
	{
		Mix_Chunk* chunk;
		Sample(Mix_Chunk* chunk) : chunk(chunk) { }
		~Sample(void) { Mix_FreeChunk(chunk); chunk = nullptr; }
	};

	struct Music
	{
		Mix_Music* music;
		Music(Mix_Music* music) : music(music) { }
		~Music(void) { Mix_FreeMusic(music); music = nullptr; }
	};

	class AudioCache
	{
	private:
		const std::string sample_dir;
		const std::string music_dir;
		std::unordered_map<uint32_t, std::unique_ptr<Sample>> sample_map;
		std::unordered_map<uint32_t, std::unique_ptr<Music>> music_map;

		std::unique_ptr<Sample> load_sample(const std::string& filename);
		std::unique_ptr<Music> load_music(const std::string& filename);

	public:
		AudioCache(const std::string& sample_dir, const std::string& music_dir)
			: sample_dir(sample_dir), music_dir(music_dir) { }
		~AudioCache(void) { free_all(); }

		Sample* get_sample(const std::string& filename);
		Music* get_music(const std::string& filename);

		void free_all(void);
		void free_sample(const std::string& filename);
		void free_music(const std::string& filename);
	};
}