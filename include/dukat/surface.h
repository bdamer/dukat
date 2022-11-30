#pragma once

#include "color.h"
#include "rect.h"

namespace dukat
{
	/**
	 * Wrapper for a SDL_Surface.
	 * 
	 * Individual pixels can be accessed directly via the `read` and `write` methods.
	 * Alternatively, the `get_pixel` and `set_pixel` methods which will automatically
	 * convert individual pixels from / to 32-bit RGBA integers.
	 * Finally, individual color values can also be accessed regardless of the 
	 * underlying bit depth via the `get_color` and `set_color` methods.
	 */
	class Surface
	{
	private:
		SDL_Surface* surface;

		// Computes a pointer to a specific pixel's position in the surface.
		constexpr uint8_t* raw_ptr(int x, int y) const { 
			return static_cast<uint8_t*>(surface->pixels) + y * surface->pitch + x * surface->format->BytesPerPixel;  
		}
		// Maps raw color to SDL color.
		SDL_Color map_raw(uint32_t c) const;

	public:
		Surface(int width, int height, uint32_t format);
		Surface(SDL_Surface* surface);
		Surface(Surface& s) : surface(s.surface) { s.surface = nullptr;  }
		~Surface(void);
		Surface& operator=(Surface& s) { surface = s.surface; s.surface = nullptr; return *this; }

		int width(void) const { return surface->w; }
		int height(void) const { return surface->h; }
		int bpp(void) const { return surface->format->BitsPerPixel; }
		SDL_Surface* get_surface(void) const { return surface; }

		// Read and write colors, independent of raw format.
		Color get_color(int x, int y) const;
		void set_color(int x, int y, const Color& color) { set_pixel(x, y, raw_color(color)); }
		// Accesses individual pixels for all supported surfaces, converting color value to uint32_t.
		uint32_t get_pixel(int x, int y) const;
		void set_pixel(int x, int y, uint32_t p);
		// Access to individual pixel values in returned in the underlying raw format.
		void read(int x, int y, uint32_t& c) const;
		void write(int x, int y, uint32_t c);
		void read(int x, int y, uint16_t& c) const;
		void write(int x, int y, uint16_t c);
		void read(int x, int y, uint8_t& c) const;
		void write(int x, int y, uint8_t c);

		// Replaces a specific color with another.
		void replace(const Color& src_color, const Color& dest_color);
		/// Replaces each source color with corresponding destination color.
		void replace(const std::vector<Color>& src_colors, const std::vector<Color>& dest_colors);

		// Returns raw color value for this surface for a given color.
		inline uint32_t raw_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff) const { return SDL_MapRGBA(surface->format, r, g, b, a); }
		inline uint32_t raw_color(const SDL_Color& color) const { return SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a); }
		uint32_t raw_color(const Color& color) const;
		
		// Returns pixel storage format and data type for OpenGL.
		void query_pixel_format(GLenum & format, GLenum & type) const;
		// Converts this surface to the specific pixel format.
		void convert_format(uint32_t format);

		// Flips surface 
		void flip_vertical(void);
		void flip_horizontal(void);

		// Applies a transformation to each pixel of the surface. Only supports 32 bit surfaces.
		void apply(const std::function<void(int x, int y, SDL_Surface* s, uint32_t& p)>& f);
	};

	// Creates a new surface for an image file.
	std::unique_ptr<Surface> load_surface(const std::string& filename);
	void save_surface(const Surface&, const std::string& filename);
}
