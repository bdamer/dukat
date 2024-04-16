#pragma once

#include <cmath>
#include <climits>
#include <random>

namespace dukat
{
	static constexpr float pi = 3.14159265f;
	static constexpr float two_pi = pi * 2.0f;
	static constexpr float pi_over_two = pi / 2.0f;
	static constexpr float pi_over_three = pi / 3.0f;
	static constexpr float pi_over_four = pi /  4.0f;
	static constexpr float pi_over_six = pi / 6.0f;
	static constexpr float pi_over_eight = pi / 8.0f;
	static constexpr float one_over_pi = 1.0f / pi;
	static constexpr float one_over_two_pi = 1.0f / two_pi;
	static constexpr float small_number = 0.000001f;
	static constexpr float big_number = 1e37f;
	static constexpr float no_intersection = 1e30f;

	// Fast inverse square root.
	float inv_sqrt(float number);
		
	// "Wrap an angle in range -PI...PI by adding the correct multiple of 2 PI
	float wrap_pi(float theta);

	// "Safe" inverse trig functions

	// Same as acos(x), but if x is out of range, it is "clamped" to the nearest
	// valid value. The value returned is in range 0...PI, the same as the 
	// standard C acos function.
	float safe_acos(float x);

	// Compute the sin and cosine of an angle. On some platforms, if we know
	// that we need both values, it can be computed faster than computing the 
	// two values seperately.
	inline void sin_cos(float &return_sin, float& return_cos, float theta)
	{
		// For simplicity, we'll just use the normal trig functions.
		// Not that on some platforms we may be able to do better.
		return_sin = std::sin(theta);
		return_cos = std::cos(theta);
	}

	// Fast sin / cos based on lookup tables.
	float fast_sin(float value);
	float fast_cos(float value);

	// convert degrees to radians and back
	inline constexpr float deg_to_rad(float x) { return (x * pi) / 180.0f; }
	inline constexpr float rad_to_deg(float x) { return (x * 180.0f) / pi; }

	// Returns the next value >= v that is a power of 2.
	inline constexpr int next_pow_two(int v)
	{
		v--;
		v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16;
		return ++v;
	}

	// Returns the power to raise 2 to a given exponent.
	inline int get_pow_two(int exponent) 
	{
		static constexpr int twos[] = 
		{
			1 << 0,  1 << 1,  1 << 2,  1 << 3,  1 << 4,  1 << 5,  1 << 6,  1 << 7,
			1 << 8,  1 << 9,  1 << 10, 1 << 11, 1 << 12, 1 << 13, 1 << 14, 1 << 15,
			1 << 16, 1 << 17, 1 << 18, 1 << 19, 1 << 20, 1 << 21, 1 << 22, 1 << 23,
			1 << 24, 1 << 25, 1 << 26, 1 << 27, 1 << 28, 1 << 29, 1 << 30, 1 << 31
		};
		return std::lower_bound(std::begin(twos), std::end(twos), exponent) - std::begin(twos);
	}

	// Rounds r to the next integer.
	inline constexpr int round(float r)
	{
		return static_cast<int>(r > 0.0f ? (r + 0.5f) : (r - 0.5f));
	}

	// Rounds r to the next multiple of base.
	inline float round(float r, float base) 
	{
		if (base != 0.f && r != 0.f)
		{
			const auto sign = r > 0.f ? 1.f : -1.f;
			r *= sign;
			r /= base;
			const auto fixed_point = static_cast<int>(std::round(r));
			r = fixed_point * base;
			r *= sign;
		}
		return r;
	}

	// Returns the positive modulo value.
	inline constexpr int pos_mod(int i, int n)
    {
        return (i % n + n) % n;
    }

	// normalizes the value of an angle between 0 and 2 pi
	inline constexpr void normalize_angle(float& angle)
	{
		if (angle < 0)
			angle += two_pi;
		else if (angle > two_pi)
			angle -= two_pi;
	}

	// computes the smallest angle between two angles.
	inline constexpr float angle_delta(float a, float b)
	{
		float diff = a - b;
		while (diff < pi) diff += two_pi;
		while (diff > pi) diff -= two_pi;
		return diff;
	}

	template<typename T>
	inline void clamp(T& value, const T min, const T max)
	{
		if (value < min)
			value = min;
		else if (value > max)
			value = max;
	}

	template <typename T>
	inline constexpr float normalize(T value, T max_value = std::numeric_limits<T>::max())
	{
		return static_cast<float>(value) / static_cast<float>(max_value);
	}

	template <typename T>
	inline T denormalize(float value, T max_value)
	{
		return static_cast<T>(value * static_cast<float>(max_value));
	}

	template <typename T> 
	int constexpr sgn(T val)
	{
		return (T(0) < val) - (val < T(0));
	}

	// Returns linear interpolation between p and q.
	template<typename T>
	T lerp(const T& p, const T& q, float t)
	{
		return p + (q - p) * t;
	}

	// Generates distribution of values within range [min_val..max_val] into data.
	template <typename T>
	void generate_distribution(std::vector<T>& data, T min_val, T max_val)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<T> dist(min_val, max_val);
		for (auto& el : data)
			el = dist(gen);
	}
}
