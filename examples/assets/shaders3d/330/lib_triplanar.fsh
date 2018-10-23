///
// Triplanar blending
///

// Computes triplanar blend parameters based on world-space normal.
vec3 compute_blending(const in vec3 normal)
{
	vec3 blending = abs(normal);
	blending = normalize(max(blending, 0.00001)); // Force weights to sum to 1.0
	float b = (blending.x + blending.y + blending.z);
	return blending / vec3(b, b, b);
}
