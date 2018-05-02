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
		
	public:
		AnimationManager(GameBase* game) : Manager(game) { }
		~AnimationManager(void) { }
		Animation* add(std::unique_ptr<Animation> animation);
		void update(float delta);

		// Clears all active animations.
		void clear(void) { animations.clear(); }
	};
}