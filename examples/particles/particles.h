#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	struct ParticleEmitter;

	class ParticlesScene : public Scene2, public Controller
	{
	private:
		struct ParticleMode
		{
			const std::string id;
			const std::function<void(void)> init;
		};

		const int camera_width = 480;
		const int camera_height = 270;

		RenderLayer2* particle_layer;
		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;
		std::vector<ParticleMode> modes;

		int cur_mode;

		void change_particle_mode(int m);
		void update_fountain(float delta);
		void update_explosion(float delta);

	public:
		ParticlesScene(Game2* game2);
		~ParticlesScene(void) { }

		void update(float delta);
		void handle_keyboard(const SDL_Event& e);
	};
}