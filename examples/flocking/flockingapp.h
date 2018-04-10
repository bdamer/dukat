#pragma once

#include <memory>
#include <queue>
#include <dukat/dukat.h>
#include "boid.h"

namespace dukat
{
	struct Sprite;

	class FlockingScene : public Scene2, public Controller
	{
	private:
		static const int window_width = 1280;
		static const int window_height = 720;

		std::unique_ptr<Sprite> cursor;
		RenderLayer2* particle_layer;
		RenderLayer2* main_layer;
		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;
		std::vector<Boid> boids;

		void add_boid(const Vector2& pos, bool predator = false);

	public:
		FlockingScene(Game2* game);
		~FlockingScene(void) { }

		void handle_keyboard(const SDL_Event& e);
		void update(float delta);
	};
}