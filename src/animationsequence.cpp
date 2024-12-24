#include "stdafx.h"
#include <dukat/animationsequence.h>

namespace dukat
{
	void AnimationSequence::step(float delta)
	{
		if (paused)
			return;

		auto next = keys.begin() + next_key;
		// next animation key
		while (time >= next->index)
		{
			if (next->value != nullptr)
				next->value(); // invoke callback
			next_key++;
			++next;
			// check if we've reached the last key
			if (next == keys.end())
			{
				if (loop)
					start(); // restart
				break;
			}
		}
		time += delta;
	}

	bool AnimationSequence::is_done(void) const
	{
		return next_key >= (keys.end() - keys.begin());
	}

	AnimationSequence& AnimationSequence::then_do(const Callback& callback)
	{
		auto index = keys.size() > 0 ? keys.back().index : 0.0f;
		keys.push_back(AnimationKey<Callback>{ index, callback });
		return *this;
	}

	AnimationSequence& AnimationSequence::wait_for(float delay, const Callback& callback)
	{
		auto index = delay + (keys.size() > 0 ? keys.back().index : 0.0f);
		keys.push_back(AnimationKey<Callback>{ index, callback });
		return *this;
	}
}