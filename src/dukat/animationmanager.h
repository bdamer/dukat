#pragma once

#include <memory>
#include <list>
#include "animation.h"

namespace dukat
{
	class AnimationManager
	{
	private:
		std::list<std::unique_ptr<Animation>> animations;
		
	public:
		AnimationManager(void) { }
		~AnimationManager(void) { }
		Animation* add(std::unique_ptr<Animation> animation);
		void update(float delta);
	};
}