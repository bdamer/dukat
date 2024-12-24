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
	
		void start(void) override { time = 0.0f; next_key = 0; }
		void stop(void) override { next_key = keys.end() - keys.begin(); loop = false; }
		void pause(void) override { paused = true; }
		void resume(void) override { paused = false; }
		void step(float delta) override;

		void set_loop(bool loop) { this->loop = loop; }
		bool is_loop(void) const { return loop; }
		bool is_running(void) const override { return next_key > -1 && !is_done(); }
		bool is_done(void) const override;

		AnimationSequence& then_do(const Callback& callback);
		AnimationSequence& wait_for(float delay, const Callback& callback = nullptr);
	};	
}