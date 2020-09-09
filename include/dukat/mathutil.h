#pragma once

#include <cmath>
#include <climits>

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
	inline float deg_to_rad(float x) { return (x*pi)/180.0f; }
	inline float rad_to_deg(float x) { return (x*180.0f)/pi; }

	// Returns the next value greater than v that is a power of 2.
	inline int next_pow_two(int v)
	{
		v--;
		v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16;
		return ++v;
	}

	// Rounds v to the next integer.
	inline int round(float r) 
	{
		return static_cast<int>(r > 0.0f ? (r + 0.5f) : (r - 0.5f));
	}

	// Returns the positive modulo value.
	inline int pos_mod(int i, int n)
    {
        return (i % n + n) % n;
    }

	// normalizes the value of an angle between 0 and 2 pi
	inline void normalize_angle(float& angle)
	{
		if (angle < 0)
			angle += two_pi;
		else if (angle > two_pi)
			angle -= two_pi;
	}

	// computes the smallest angle between two angles.
	inline float angle_delta(float a, float b)
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

	inline float normalize(int value, int max_value = INT_MAX)
	{
		return static_cast<float>(value) / static_cast<float>(max_value);
	}

	inline float normalize(short value, short max_value = SHRT_MAX)
	{
		return static_cast<float>(value) / static_cast<float>(max_value);
	}

	inline float normalize(char value, char max_value = CHAR_MAX)
	{
		return static_cast<float>(value) / static_cast<float>(max_value);
	}

	inline float normalize(unsigned char value)
	{
		return static_cast<float>(value) / 255.0f;
	}

	template <typename T> 
	int sgn(T val) 
	{
		return (T(0) < val) - (val < T(0));
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