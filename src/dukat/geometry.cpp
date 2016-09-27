#include "stdafx.h"
#include "geometry.h"
#include "mathutil.h"

namespace dukat
{
	void circle_table(std::vector<float>& sint, std::vector<float>& cost, int n, bool half_circle)
	{
		// Table size, the sign of n flips the circle direction
		int size = abs(n);
		// Determine the angle between samples
		float angle = (half_circle ? 1 : 2) * pi / (float)((n == 0) ? 1 : n);

		// Compute cos and sin around the circle
		sint[0] = 0.0f;
		cost[0] = 1.0f;
		for (int i = 1; i < size; i++) 
		{
			sint[i] = sin(angle * i);
			cost[i] = cos(angle * i);
		}

		if (half_circle) 
		{
			sint[size] = 0.0f;  // sin PI
			cost[size] = -1.0f;  // cos PI
		}
		else
		{
			// Last sample is duplicate of the first (sin or cos of 2 PI)
			sint[size] = sint[0];
			cost[size] = cost[0];
		}
	}

	std::vector<float> generate_sphere(float radius, int slices, int stacks, bool inverted) {
		std::vector<float> res;
		// number of unique vertices
		if (slices == 0 || stacks < 2) {
			// nothing to generate
			return res;
		}
		int num_vert = slices * (stacks - 1) + 2;
		if (num_vert > 65535) {
			// limit of glushort, thats 256*256 subdivisions, should be enough in practice.
			throw std::runtime_error("Too many slices or stacks requested.");
		}		
		res.resize(num_vert * 6);

		// precompute values on unit circle
		std::vector<float> sint1(slices + 1);
		std::vector<float> cost1(slices + 1);
		std::vector<float> sint2(stacks + 1);
		std::vector<float> cost2(stacks + 1);
		circle_table(sint1, cost1, -slices, false);
		circle_table(sint2, cost2, stacks, true);

		// indexes into vertex and normal arrays
		int idx = 0;

		// multiplier in case the sphere should be inverted
		float ns = inverted ? -1.0f : 1.0f;

		// top
		res[idx++] = ns * 0.0f;
		res[idx++] = ns * radius;
		res[idx++] = ns * 0.0f;
		res[idx++] = ns * 0.0f;
		res[idx++] = ns * 1.0f;
		res[idx++] = ns * 0.0f;

		// each stack
		float x, y, z;
		for (int i = 1; i < stacks; i++) {
			for (int j = 0; j < slices; j++) {
				x = cost1[j] * sint2[i];
				y = cost2[i];
				z = -sint1[j] * sint2[i];
				res[idx++] = ns * x * radius;
				res[idx++] = ns * y * radius;
				res[idx++] = ns * z * radius;
				res[idx++] = ns * x;
				res[idx++] = ns * y;
				res[idx++] = ns * z;
			}
		}

		// bottom
		res[idx++] = ns * 0.0f;
		res[idx++] = ns * -radius;
		res[idx++] = ns * 0.0f;
		res[idx++] = ns * 0.0f;
		res[idx++] = ns * -1.0f;
		res[idx++] = ns * 0.0f;

		return res;
	}
}