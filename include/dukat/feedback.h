#pragma once

#include <memory>
#include <vector>

namespace dukat
{
	struct FeedbackValue
	{
		float low;
		float high;
	};

	struct FeedbackKey
	{
		const float index; // time index
		const FeedbackValue value; // target value

		FeedbackKey(float index, const FeedbackValue& value) : index(index), value(value) { }
		FeedbackKey(float index, float low, float high) : index(index), value(FeedbackValue{ low, high }) { }
	};

	struct FeedbackSequence
	{
		const float duration; // total duration, if <= 0 looping sequence
		const std::vector<FeedbackKey> keys;

		float runtime; // how long this sequence has run
		float index; // current time index
		int cur_key; // index of current key
		int next_key; // index of next key

		FeedbackSequence(float duration, const std::vector<FeedbackKey>& keys)
			: duration(duration), keys(keys), runtime(0.0f), index(0.0f), cur_key(0), next_key(1)
		{
		}

		void update(float delta);
		bool is_done(void) const { return duration > 0.0f && runtime > duration; }
	};

	inline std::unique_ptr<FeedbackSequence> make_sequence(float duration, float low, float high)
	{
		std::vector<FeedbackKey> keys = {
			FeedbackKey{ 0.0f, low, high },
			FeedbackKey{ duration, 0.0f, 0.0f },
		};
		return std::make_unique<FeedbackSequence>(duration, keys);
	}
}