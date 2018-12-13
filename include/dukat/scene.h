#pragma once

#include "gamebase.h"

namespace dukat
{
	// Abstract interface for a scene. The application keeps
	// track of a stack of scenes, with the top-most scene controlling
	// what is currently rendered.
	class Scene
	{
	private:
		bool active;

	public:
		// Called once to initialize the scene.
		Scene(void) : active(false) { }
		// Called once to destroy the scene.
		virtual ~Scene(void) { }

		// Called whenever the scene becomes active.
		virtual void activate(void) { active = true; }
		// Called once per frame to update the scene.
		virtual void update(float delta) = 0;
		// Called once per frame to render the scene.
		virtual void render(void) = 0;
		// Called whenever the scene is deactivated.
		virtual void deactivate(void) { active = false; }

		bool is_active(void) const { return active; }
	};
}