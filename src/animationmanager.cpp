#include "stdafx.h"
#include <dukat/animationmanager.h>

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

	void AnimationManager::cancel(Animation* animation)
	{
		auto it = std::find_if(animations.begin(), animations.end(), [animation](const std::unique_ptr<Animation>& anim) {
			return animation == anim.get();
		});
		if (it != animations.end())
			animations.erase(it);
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