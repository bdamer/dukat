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

	template <typename T>
	class Animation
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
		// called when animation is done
		std::function<void(void)> callback;

	public:
		// Creates a new animation for the attribute provided.
		Animation(T* attribute) : attribute(attribute), next_key(-1), loop(false) { }
		// Creates a new animation with a single animation key specified by time and value.
		Animation(T* attribute, float time, T value, bool loop = false)
			: attribute(attribute), next_key(-1), loop(loop) { add_key({ time, value }); }
		~Animation(void) { }

		void set_callback(const std::function<void(void)>& callback) { this->callback = callback; }
		void set_loop(bool loop) { this->loop = loop; }
		bool is_loop(void) const { return loop; }
		bool is_running(void) const { return next_key > -1 && !is_done(); }
		bool is_done(void) const { return (keys.begin() + next_key) == keys.end(); }

		void add_key(const AnimationKey<T>& key) { keys.push_back(key); }

		void start(void);
		void stop(void);
		void step(float delta);
	};

	template<typename T>
	inline void Animation<T>::start(void)
	{
		time = 0.0f;
		next_key = 0;
		// compute delta used by continuous animations
		auto next = keys.begin();
		value_delta = (next->value - *attribute) / next->index;
	}

	template<typename T>
	inline void Animation<T>::stop(void)
	{
		next_key = keys.end() - keys.begin();
	}

	template<typename T>
	inline void Animation<T>::step(float delta)
	{
		auto next = keys.begin() + next_key;
		if (next->mode == AnimationKey<T>::Continuous)
		{
			*attribute += value_delta * delta;
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