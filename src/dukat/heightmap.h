#pragma once

#include <vector>

namespace dukat
{
    struct Rect;
    class Surface;
	class HeightMapGenerator;
	class Ray3;

    class HeightMap
    {
    public:
        struct Level
        {
            int index; // level index
            int size; // width and height of this level
            std::vector<GLfloat> data; // Normalized elevation data [0..1]

            Level(void) : index(-1), size(0) { }
            Level(int index, int size) : index(index), size(size), data(size * size) { }

            float& operator[](int index) { return data[index]; }
            float operator[](int index) const { return data[index]; }
            float min(void) const { return *std::min_element(data.begin(), data.end()); }
            float max(void) const { return *std::max_element(data.begin(), data.end()); }
        };

    private:
        const int num_levels; // number of terrain levels contained in this map
        int level_size; // width / height of each level
		float scale_factor; // Scale factor used to compute grid height from normalized elevation data. 
        std::vector<Level> levels; // height level data

        // Generates levels 1..n based on level 0
        void generate_levels(void);

    public:
        HeightMap(int num_levels, float scale_factor = 1.0f) 
            : num_levels(num_levels), scale_factor(scale_factor), level_size(0) { }
        ~HeightMap(void) { }

		// Loads height data from a 16-bit grayscale PNG file.
		void load(const std::string& filename);
		// Saves height data as 16-bit grayscale PNG file.
		void save(const std::string& filename) const;
        // Allocates a blank heightmap of a given size.
        void allocate(int level_size);
		// Generates random fractal terrain.
		void generate(int level_size, const HeightMapGenerator& generator);

        // Copies data at a given level within a rect into a provided buffer. If the
        // buffer is not large enough to contain the requested rect, partial data
        // will be returned.
        void get_data(int level, const Rect& rect, std::vector<GLfloat>& buffer) const;
        // Returns reference to a level for direct access.
        Level& get_level(int level) { return levels[level]; }

		// Returns the normalized elevation at a given set of coordinates and level.
		float get_elevation(int x, int y, int level) const;
        // Sets the elevation at a coordinate and level without updating other levels.
        void set_elevation(int x, int y, int level, float z);

		// Samples normalized elevation at a given set of coordinates, performing 
		// bilinear sampling if necessary.
		float sample(int level, float x, float y) const;

		// Tests for intersection with a ray. Will return the distance of intersection or no_intersection.
		float intersect_ray(const Ray3& ray, float min_t = 0.0f, float max_t = 1000.0f) const;

        // Getters and setters
        float get_scale_factor(void) const { return scale_factor; }
        void set_scale_factor(float factor) { this->scale_factor = factor; }
	};
}