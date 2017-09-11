#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class Player;

	class LightingScene : public Scene, public Controller
	{
	private:
		Game3* game;
		MeshGroup object_meshes;
		MeshGroup overlay_meshes;
		MeshInstance* info_mesh;
		OrbitalLight light;
		bool animate_light;

	public:
		LightingScene(Game3* game);
		~LightingScene(void) {}

		void update(float delta);
		void render(void);
		bool handle_event(const SDL_Event& e);
		bool handle_keyboard(const SDL_Event& e);
	};
}