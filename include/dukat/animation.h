#pragma once

#include <functional>
#include <vector>

namespace dukat
{
	template <typename T>
	struct AnimationKey
	{
		enum Mode
		{
			Continuous, Discrete
		};

		float index;
		T value;
		Mode mode;

		AnimationKey(float index, T value, Mode mode = Mode::Continuous) : index(index), value(value), mode(mode) { }
	};

	// Virtual interface used to store different animations in a single container.
	class Animation
	{
	public:
		Animation(void) { }
		virtual ~Animation(void) { }
		virtual bool is_running(void) const = 0;
		virtual bool is_done(void) const = 0;
		virtual void start(void) = 0;
		virtual void stop(void) = 0;
		virtual void pause(void) = 0;
		virtual void resume(void) = 0;
		virtual void step(float delta) = 0;
	};

	template <typename T>
	class ValueAnimation : public Animation
	{
	private:
		// animation keys
		std::vector<AnimationKey<T>> keys;
		int next_key;
		// time since animation was started
		float time;
		// target attribute
		T* attribute;
		// change to attribute per second
		T value_delta;
		// if true will loop animation
		bool loop;
		// if true, animation is paused
		bool paused;
		// called when animation is done
		std::function<void(void)> callback;

	public:
		// Creates a new animation for the attribute provided.
		ValueAnimation(T* attribute) : attribute(attribute), next_key(-1), loop(false), paused(false) { }
		// Creates a new animation with a single animation key specified by time and value.
		ValueAnimation(T* attribute, float time, T value, bool loop = false)
			: attribute(attribute), next_key(-1), loop(loop), paused(false) { add_key({ time, value }); }
		~ValueAnimation(void) { }

		void set_callback(const std::function<void(void)>& callback) { this->callback = callback; }
		void set_loop(bool loop) { this->loop = loop; }
		bool is_loop(void) const { return loop; }
		bool is_running(void) const { return next_key > -1 && !is_done(); }
		bool is_done(void) const { return (keys.begin() + next_key) == keys.end(); }

		void add_key(const AnimationKey<T>& key) { keys.push_back(key); }

		void start(void);
		void stop(void);
		void pause(void) { paused = true; }
		void resume(void) { paused = false; }
		void step(float delta);
	};

	template<typename T>
	inline void ValueAnimation<T>::start(void)
	{
		time = 0.0f;
		next_key = 0;
		// compute delta used by continuous animations
		auto next = keys.begin();
		value_delta = (next->value - *attribute) / next->index;
	}

	template<typename T>
	inline void ValueAnimation<T>::stop(void)
	{
		next_key = keys.end() - keys.begin();
	}

	template<typename T>
	inline void ValueAnimation<T>::step(float delta)
	{
		if (paused)
			return;

		auto next = keys.begin() + next_key;
		if (next->mode == AnimationKey<T>::Continuous)
		{
			*attribute += static_cast<T>(value_delta * delta);
		}
		// next animation key
		if (time >= next->index)
		{
			*attribute = next->value;
			next_key++;
			++next;
			// check if we've reached the last key
			if (next == keys.end())
			{
				if (callback)
					callback();
				if (loop)
					start(); // restart
			}
			else
			{
				// compute new delta for continuous animations
				value_delta = (next->value - *attribute) / (next->index - time);
			}
		}
		time += delta;
	}
}