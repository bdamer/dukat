#pragma once

#include <vector>

namespace dukat
{
    struct Rect;
    class Surface;

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
		const float scale_factor;
        int level_size;
        std::vector<HeightMapLevel> levels;

        void generate_levels(void);
        // Loads height data from a 16-bit grayscale PNG file.
        void load(const std::string& filename);

    public:
        HeightMap(int num_levels, const std::string& filename, float scale_factor);
        ~HeightMap(void) { }

		float get_scale_factor(void) const { return scale_factor; }

        // Copies data at a given level within a rect into a provided buffer. If the
        // buffer is not large enough to contain the requested rect, partial data
        // will be returned.
        void get_data(int level, const Rect& rect, std::vector<GLfloat>& buffer) const;
    
		// Returns the elevation at a given set of coordinates and level.
		float get_elevation(int level, int x, int y) const;
	};
}