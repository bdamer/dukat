#pragma once

#include <functional>
#include <vector>

namespace dukat
{
	// Animation Key definition
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
		typedef std::function<void(void)> Callback;
		uint8_t group; // animation group

		Animation(void) : group(0u) { }
		virtual ~Animation(void) { }
		virtual bool is_running(void) const = 0;
		virtual bool is_done(void) const = 0;
		virtual void start(void) = 0;
		virtual void stop(void) = 0;
		virtual void pause(void) = 0;
		virtual void resume(void) = 0;
		virtual void step(float delta) = 0;
	};

	// Animation over a single value.
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
		Callback callback;

	public:
		// Creates a new animation for the attribute provided.
		ValueAnimation(T* attribute) : next_key(-1), time(0.0f), attribute(attribute), value_delta(), loop(false), paused(false), callback(nullptr) { }
		// Creates a new animation with a single animation key specified by time and value.
		ValueAnimation(T* attribute, float time, T value, bool loop = false)
			: next_key(-1) , time(0.0f), attribute(attribute), loop(loop), paused(false), callback(nullptr) {
			add_key({ time, value });
		}
		~ValueAnimation(void) { }

		void set_callback(const Callback& callback) { this->callback = callback; }
		void set_loop(bool loop) { this->loop = loop; }
		bool is_loop(void) const { return loop; }
		bool is_running(void) const override { return next_key > -1 && !is_done(); }
		bool is_done(void) const override { return next_key >= (keys.end() - keys.begin()); }
		void add_key(const AnimationKey<T>& key) { keys.push_back(key); }

		void start(void) override;
		void stop(void) override { next_key = keys.end() - keys.begin(); loop = false; }
		void pause(void) override { paused = true; }
		void resume(void) override { paused = false; }
		void step(float delta) override;
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

	// Animation over multiple values.
	template <typename T, std::size_t S>
	class MultiValueAnimation : public Animation
	{
	private:
		// animation keys
		std::vector<AnimationKey<std::array<T, S>>> keys;
		int next_key;
		// time since animation was started
		float time;
		// target attribute
		std::array<T*, S> attributes;
		// change to attribute per second
		std::array<T, S> value_deltas;
		// if true will loop animation
		bool loop;
		// if true, animation is paused
		bool paused;
		// called when animation is done
		Callback callback;

	public:
		// Creates a new animation for the attribute provided.
		MultiValueAnimation(const std::array<T*, S>& attributes) : next_key(-1), time(0.0f), attributes(attributes), value_deltas(), loop(false), paused(false), callback(nullptr) { }
		// Creates a new animation with a single animation key specified by time and value.
		MultiValueAnimation(const std::array<T*, S>& attributes, float time, const std::array<T, S>& values, bool loop = false)
			: next_key(-1), time(0.0f), attributes(attributes), loop(loop), paused(false), callback(nullptr) {
			add_key({ time, values });
		}
		~MultiValueAnimation(void) { }

		void set_callback(const Callback& callback) { this->callback = callback; }
		void set_loop(bool loop) { this->loop = loop; }
		bool is_loop(void) const { return loop; }
		bool is_running(void) const { return next_key > -1 && !is_done(); }
		bool is_done(void) const { return (keys.begin() + next_key) == keys.end(); }
		void add_key(const AnimationKey<std::array<T, S>>& key) { keys.push_back(key); }

		void start(void);
		void stop(void) { next_key = keys.end() - keys.begin(); loop = false; }
		void pause(void) { paused = true; }
		void resume(void) { paused = false; }
		void step(float delta);
	};

	template <typename T, std::size_t S>
	inline void MultiValueAnimation<T, S>::start(void)
	{
		time = 0.0f;
		next_key = 0;
		// compute delta used by continuous animations
		auto next = keys.begin();
		for (auto i = 0u; i < S; i++)
			value_deltas[i] = (next->value[i] - *attributes[i]) / next->index;
	}

	template<typename T, std::size_t S>
	inline void MultiValueAnimation<T, S>::step(float delta)
	{
		if (paused)
			return;

		auto next = keys.begin() + next_key;
		if (next->mode == AnimationKey<std::array<T, S>>::Continuous)
		{
			for (auto i = 0u; i < S; i++)
				*attributes[i] += static_cast<T>(value_deltas[i] * delta);
		}
		// next animation key
		if (time >= next->index)
		{
			for (auto i = 0u; i < S; i++)
				*attributes[i] = next->value[i];
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
				for (auto i = 0u; i < S; i++)
					value_deltas[i] = (next->value[i] - *attributes[i]) / (next->index - time);
			}
		}
		time += delta;
	}
}