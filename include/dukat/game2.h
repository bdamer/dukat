#pragma once

#include <memory>
#include "gamebase.h"
#include "renderer2.h"
#include "scene2.h"

namespace dukat
{
	// Game implementation using the 2D subsystem.
	class Game2 : public GameBase
	{
	protected:
		std::unique_ptr<Renderer2> renderer;
		virtual void update(float delta);
		void toggle_debug(void);

	public:
		Game2(Settings& settings);
		virtual ~Game2(void);
		std::unique_ptr<TextMeshInstance> create_text_mesh(void);

		Renderer2* get_renderer(void) const { return renderer.get(); }
	};
}