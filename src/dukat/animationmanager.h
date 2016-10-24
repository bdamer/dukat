#pragma once

#include <memory>
#include <list>
#include "animation.h"

namespace dukat
{
	class AnimationManager
	{
	private:
		std::list<std::unique_ptr<Animation<float>>> animations;

	public:
		AnimationManager(void) { }
		~AnimationManager(void) { }
		Animation<float>* add(std::unique_ptr<Animation<float>> animation);
		void update(float delta);
	};
}