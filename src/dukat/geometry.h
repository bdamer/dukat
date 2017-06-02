#pragma once

#include "mathutil.h"

namespace dukat
{
	// Compute lookup table of cos and sin values forming a circle segment of angle a.
	// Based on fghCircleTable from freeglut.
	// 
	// Notes:
	//  - The size of the table is (n+1) to form a connected loop
	//  - The last entry is exactly the same as the first
	//  - The sign of n can be flipped to get the reverse loop
	void circle_table(std::vector<float>& sint, std::vector<float>& cost, int n, float a = two_pi);

 	// Generates vertices and normals for a sphere of a given radius. Based on 
	// fghGenerateSphere from freeglut.
	//
	// Notes:
	//  - The result vector will contain 3 position and 3 normal elements for 
	//    each vertex.
	std::vector<float> generate_sphere(float radius, int slices, int stacks, bool inverted = false);

	// Generates vertices and normals for a dome of a given radius. Based on
	// fghGenerateSpehere from freeglut.
	//
	// Notes:
	//  - The result vector will contain 3 position and 3 normal elements for 
	//    each vertex.
	std::vector<float> generate_dome(float radius, int slices, int stacks, bool inverted = false);
}