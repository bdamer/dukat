#pragma once

#include <cstdlib>
#include <cmath>
#include <climits>

namespace dukat
{
	const float pi = 3.14159265f;
	const float two_pi = pi * 2.0f;
	const float pi_over_two = pi / 2.0f;
	const float pi_over_three = pi / 3.0f;
	const float pi_over_four = pi /  4.0f;
	const float pi_over_six = pi / 6.0f;
	const float pi_over_eight = pi / 8.0f;
	const float one_over_pi = 1.0f / pi;
	const float one_over_two_pi = 1.0f / two_pi;
	const float small_number = 0.000001f;
	const float big_number = 1e37f;
	const float no_intersection = 1e30f;

	// Fast inverse square root.
	extern float inv_sqrt(float number);
		
	// "Wrap an angle in range -PI...PI by adding the correct multiple of 2 PI
	extern float wrap_pi(float theta);

	// "Safe" inverse trig functions

	// Same as acos(x), but if x is out of range, it is "clamped" to the nearest
	// valid value. The value returned is in range 0...PI, the same as the 
	// standard C acos function.
	extern float safe_acos(float x);

	// Compute the sin and cosine of an angle. On some platforms, if we know
	// that we need both values, it can be computed faster than computing the 
	// two values seperately.
	inline void sin_cos(float &return_sin, float& return_cos, float theta)
	{
		// For simplicity, we'll just use the normal trig functions.
		// Not that on some platforms we may be able to do better.
		return_sin = ::sin(theta);
		return_cos = cos(theta);
	}

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
		return (int)(r > 0.0f ? (r + 0.5f) : (r - 0.5f));
	}

	// Returns the positive modulo value.
	inline int pos_mod(int i, int n)
    {
        return (i % n + n) % n;
    }

	inline float randf(float min, float max)
	{
		return min + (max - min) * (float)(rand() % 1000) / 1000.0f;
	}

	inline int randi(int min, int max)
	{
		return min + rand() % (max - min);
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

	inline void clamp(float& value, float min, float max)
	{
		if (value < min)
			value = min;
		else if (value > max)
			value = max;
	}

	inline float normalize(int value, int max_value = INT_MAX)
	{
		return ((float)value / (float)max_value);
	}

	inline float normalize(short value, short max_value = SHRT_MAX)
	{
		return ((float)value / (float)max_value);
	}

	inline float normalize(char value, char max_value = CHAR_MAX)
	{
		return ((float)value / (float)max_value);
	}

	inline float normalize(unsigned char value)
	{
		return ((float)value / 255.0f);
	}
}