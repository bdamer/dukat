#include "stdafx.h"
#include <dukat/animationmanager.h>

namespace dukat
{
	Animation* AnimationManager::add(std::unique_ptr<Animation> animation)
	{
		auto anim = animation.get();
		anim->group = active_group;
		animations.push_back(std::move(animation));
		if (!anim->is_running())
			anim->start();
		return anim;
	}

	void AnimationManager::cancel(Animation* animation)
	{
		auto it = std::find_if(animations.begin(), animations.end(), [animation](const std::unique_ptr<Animation>& anim) {
			return animation == anim.get();
		});
		if (it != animations.end())
			(*it)->stop(); // don't erase here - update will take care of that
	}

	void AnimationManager::update(float delta)
	{
		for (auto it = animations.begin(); it != animations.end(); )
		{
			const auto& a = (*it);
			// only process animations of group 0 or active group
			if (a->group == active_group || a->group == 0)
			{
				if (a->is_done())
				{
					it = animations.erase(it);
					continue;
				}
				else
				{
					a->step(delta);
				}
			}
			++it;
		}
	}
}