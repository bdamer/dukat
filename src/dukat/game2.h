#pragma once

#include <memory>
#include "gamebase.h"
#include "renderer2.h"
#include "scene2.h"

namespace dukat
{
	class CollisionManager2;

	// Game implementation using the 2D subsystem.
	class Game2 : public GameBase
	{
	protected:
		std::unique_ptr<CollisionManager2> collision_manger;
		std::unique_ptr<Renderer2> renderer;
		void toggle_debug(void);

	public:
		Game2(Settings& settings);
		virtual ~Game2(void);

		CollisionManager2* get_collisions(void) const { return collision_manger.get(); }
		Renderer2* get_renderer(void) const { return renderer.get(); }
	};
}