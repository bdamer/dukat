#pragma once

#include <dukat/controller.h>
#include <dukat/scene.h>
#include <dukat/meshgroup.h>
#include <dukat/orbitallight.h>

namespace dukat
{
	class Game3;

	class GameScene : public Scene, public Controller
	{
	private:
		Game3* game;
		MeshGroup scene_meshes;
		MeshGroup overlay_meshes;
		MeshInstance* info_mesh;
		Vector3 camera_target;
		OrbitalLight light;
		bool enable_lighting;

	public:
		GameScene(Game3* game);
		~GameScene(void) { }

		void activate(void);
		void update(float delta);
		bool handle_event(const SDL_Event& e);
		bool handle_keyboard(const SDL_Event& e);
		void render(void);
	};
}