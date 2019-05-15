#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class ParticlesScene : public Scene2, public Controller
	{
	private:
		struct ParticleMode
		{
			const std::string id;
			const float particle_rate; // number of particles to emit per second
			float max_change; // max change of emitter angle per second			
			const std::function<void(void)> init;
			const std::function<void(float)> update;
		};

		struct Emitter
		{
			const Vector2 pos;
			float cur_angle; // current angle of particle emitter
			float max_angle; // max angle in +/- direction
		};

		const int camera_width = 480;
		const int camera_height = 270;

		RenderLayer2* particle_layer;
		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;
		std::vector<ParticleMode> modes;

		int cur_mode;
		float particle_accu; // accumulator for particle counts
		std::vector<Emitter> emitters;

		void change_particle_mode(int m);
		void update_fire(float delta);
		void update_smoke(float delta);
		void update_fountain(float delta);
		void update_explosion(float delta);

	public:
		ParticlesScene(Game2* game2);
		~ParticlesScene(void) { }

		void update(float delta);
		void handle_keyboard(const SDL_Event& e);
	};
}