#pragma once

#include <vector>

namespace dukat
{
    struct Rect;
    class Surface;
	class HeightMapGenerator;
	class Ray3;

    struct HeightMapLevel
    {
        int index; 
        int size; // width and height of this level
        std::vector<GLfloat> data; // Normalized elevation data [0..1]

        HeightMapLevel(void) : index(-1), size(0) { }
        HeightMapLevel(int index, int size) : index(index), size(size), data(size * size) { }
    };

    class HeightMap
    {
    private:
        const int num_levels;
		// Scale factor used to compute grid height from normalized elevation data. 
		float scale_factor;
        int level_size;
        std::vector<HeightMapLevel> levels;

        void generate_levels(void);

    public:
        HeightMap(int num_levels) : num_levels(num_levels) { }
        ~HeightMap(void) { }

		// Loads height data from a 16-bit grayscale PNG file.
		void load(const std::string& filename, float scale_factor);
		// Saves height data as 16-bit grayscale PNG file.
		void save(const std::string& filename) const;
		// Generates random fractal terrain.
		void generate(int level_size, float scale_factor, const HeightMapGenerator& generator);

		float get_scale_factor(void) const { return scale_factor; }

        // Copies data at a given level within a rect into a provided buffer. If the
        // buffer is not large enough to contain the requested rect, partial data
        // will be returned.
        void get_data(int level, const Rect& rect, std::vector<GLfloat>& buffer) const;
    
		// Returns the normalized elevation at a given set of coordinates and level.
		float get_elevation(int level, int x, int y) const;

		// Samples normalized elevation at a given set of coordinates, performing 
		// bilinear sampling if necessary.
		float sample(int level, float x, float y) const;

		// Tests for intersection with a ray. Will return the distance of intersection or no_intersection.
		float intersect_ray(const Ray3& ray, float min_t = 0.0f, float max_t = 1000.0f) const;
	};
}