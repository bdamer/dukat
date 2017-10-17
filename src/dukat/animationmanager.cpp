#include "stdafx.h"
#include "animationmanager.h"

namespace dukat
{
	Animation* AnimationManager::add(std::unique_ptr<Animation> animation)
	{
		auto anim = animation.get();
		animations.push_back(std::move(animation));
		if (!anim->is_running())
		{
			anim->start();
		}
		return anim;
	}

	void AnimationManager::update(float delta)
	{
		for (auto it = animations.begin(); it != animations.end(); )
		{
			(*it)->step(delta);
			if ((*it)->is_done())
			{
				it = animations.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}