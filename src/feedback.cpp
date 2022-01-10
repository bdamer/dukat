#include "stdafx.h"
#include <dukat/feedback.h>
#include <dukat/log.h>

namespace dukat
{
	void FeedbackSequence::update(float delta)
	{
		runtime += delta;
		index += delta;

		const auto& next = keys[next_key];
		// next animation key
		if (index >= next.index)
		{
			cur_key = next_key;
			++next_key;

			// check if we've reached the last key
			if (next_key == keys.size())
			{
				next_key = 0;
				index -= next.index; // reset time index
			}
		}
	}
}