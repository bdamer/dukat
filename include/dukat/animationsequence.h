#pragma once

#include "animation.h"

namespace dukat
{
	class AnimationSequence : public Animation
	{
	private:		
		// animation keys
		std::vector<AnimationKey<Callback>> keys;
		int next_key;
		// time since animation was started
		float time;
		// if true will loop animation
		bool loop;
		// if true, animation is paused
		bool paused;

	public:
		AnimationSequence(void) : next_key(-1), time(0.0f), loop(false), paused(false) { }
		~AnimationSequence(void) { }
	
		void start(void) { time = 0.0f; next_key = 0; }
		void stop(void) { next_key = keys.end() - keys.begin(); }
		void pause(void) { paused = true; }
		void resume(void) { paused = false; }
		void step(float delta);

		void set_loop(bool loop) { this->loop = loop; }
		bool is_loop(void) const { return loop; }
		bool is_running(void) const { return next_key > -1 && !is_done(); }
		bool is_done(void) const { return (keys.begin() + next_key) == keys.end(); }

		AnimationSequence& then_do(const Callback& callback);
		AnimationSequence& wait_for(float delay, const Callback& callback = nullptr);
	};	
}