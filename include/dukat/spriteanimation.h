#pragma once

#include <cmath>
#include <robin_hood.h>
#include <string>

namespace dukat
{
	struct SpriteAnimation
	{
		struct Sequence
		{
			int from;
			int to;
			float step;
		};

		robin_hood::unordered_map<std::string, Sequence> sequences;
		Sequence* sequence; // current sequence
		float time_idx; // current time within the sequence
		int index; // sprite index

		SpriteAnimation(void) : sequence(nullptr), time_idx(0.0f), index(0) { }

		// Moves ahead animation by given time delta
		void update(float delta) 
		{ 
			time_idx += delta; 
			index = sequence->from + static_cast<int>(std::floor(time_idx / sequence->step)); 
		}

		// Returns duration of the current animation.
		float cur_duration(void) const 
		{ 
			return sequence != nullptr ? sequence->step * static_cast<float>(1 + sequence->to - sequence->from) : 0.0f; 
		}

		// Returns time remaining for the current animation.
		float time_remaining(void) const 
		{
			return cur_duration() - time_idx; 
		}

		void reset_sequence(void) 
		{ 
			index = sequence->from;
			time_idx = 0.0f; 
		}

		void add_sequence(const std::string& id, int from, int to, float step) 
		{ 
			sequences.emplace(id, Sequence{ from, to, step }); 
		}
		
		void set_sequence(const std::string& id, bool reset_timer) 
		{
			if (sequences.count(id))
			{
				sequence = &sequences.at(id);
				index = sequence->from;
			}
			else
			{
				sequence = nullptr;
			}
			if (reset_timer) time_idx = 0.0f;
		}
		
		bool has_sequence(const std::string& id) const
		{
			return sequences.contains(id);
		}

		bool sequence_done(void) const 
		{ 
			return index > sequence->to; 
		}

		void set_sequence_step(const std::string& id, float step)
		{
			if (sequences.count(id))
				sequences.at(id).step = step;
		}
	};
}