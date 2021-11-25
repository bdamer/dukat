#pragma once

#include "gamebase.h"
#include "renderer3.h"
#include "meshgroup.h"

namespace dukat
{
	class Game3 : public GameBase
	{
	protected:
		std::unique_ptr<Renderer3> renderer;
		MeshGroup debug_meshes;

		virtual void update(float delta);
		virtual void update_debug_text(void);
		void toggle_debug(void);
		void save_screenshot(const std::string& filename);

	public:
		Game3(Settings& settings);
		virtual ~Game3(void) { }
		std::unique_ptr<TextMeshInstance> create_text_mesh(BitmapFont* font = nullptr);

		Renderer3* get_renderer(void) const { return renderer.get(); }
		MeshGroup* get_debug_meshes(void) { return &debug_meshes; }
	};
}