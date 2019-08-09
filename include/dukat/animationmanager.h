#pragma once

#include <memory>
#include <list>

#include "animation.h"
#include "manager.h"

namespace dukat
{
	class AnimationManager : public Manager
	{
	private:
		std::list<std::unique_ptr<Animation>> animations;
		uint8_t active_group;

	public:
		AnimationManager(GameBase* game) : Manager(game), active_group(0u) { }
		~AnimationManager(void) { }

		void update(float delta);

		// Add a new animation to current group.
		Animation* add(std::unique_ptr<Animation> animation);
		// Cancels an existing animation.
		void cancel(Animation* animation);
		// Clears all active animations.
		void clear(void) { animations.clear(); }
		// Set active animation group
		void set_active_group(uint8_t group) { this->active_group = group; }
	};
}