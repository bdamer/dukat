#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class Player;
	class HeatMap;
	struct OrbitalLight;

	class Game : public Game3
	{
	private:
		MeshGroup debug_meshes;
		MeshGroup object_meshes;
		MeshGroup overlay_meshes;
		MeshInstance* info_mesh;
		TextMeshInstance* mult_mesh;
		MeshInstance* observer_mesh;
		std::unique_ptr<HeatMap> heatmap;
		std::unique_ptr<OrbitalLight> light;
		Vector3 camera_target;

        int multiplier; // Simulation multiplier
		bool animate_light;

		void init(void);
		void update(float delta);
		void update_debug_text(void);
		void render(void);
		void handle_event(const SDL_Event& e);
		void handle_keyboard(const SDL_Event& e);
		void release(void);

	public:
		Game(Settings& settings) : Game3(settings), multiplier(1), animate_light(true) { }
		~Game(void) { }

		void toggle_debug(void);
	};
}