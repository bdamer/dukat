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

	std::unique_ptr<FeedbackSequence> make_sequence(float duration, float low, float high)
	{
		std::vector<FeedbackKey> keys = {
			FeedbackKey{ 0.0f, low, high },
			FeedbackKey{ duration, 0.0f, 0.0f },
		};
		return std::make_unique<FeedbackSequence>(duration, keys);
	}

	std::unique_ptr<FeedbackSequence> make_step_sequence(float duration, float on_duration, float off_duration,
		float lo_on, float lo_off, float hi_on, float hi_off)
	{
		std::vector<FeedbackKey> keys = {
			FeedbackKey{ 0.0f, lo_on, hi_on },
			FeedbackKey{ on_duration, lo_off, hi_off },
			FeedbackKey{ on_duration + off_duration, lo_on, hi_on }
		};
		return std::make_unique<FeedbackSequence>(duration, keys);
	}
}