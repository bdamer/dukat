#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class Player;
	class HeatMap;
	struct OrbitalLight;

	class HeatmapScene : public Scene, public Controller
	{
	private:
		Game3* game;
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

	public:
		HeatmapScene(Game3* game);
		~HeatmapScene(void) { }

		void update(float delta);
		void render(void);
		void handle_event(const SDL_Event& e);
		void handle_keyboard(const SDL_Event& e);
	};
}