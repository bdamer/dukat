#pragma once

#include "color.h"
#include "rect.h"

namespace dukat
{
	/**
	 * Wrapper for a 32-bit SDL surface.
	 */
	class Surface
	{
	private:
		SDL_Surface* surface;

		// Maps raw color to SDL color.
		SDL_Color map_color(SDL_PixelFormat* fmt, uint32_t c) const;

	public:
		Surface(int width, int height, uint32_t format);
		Surface(SDL_Surface* surface);
		Surface(Surface& s) : surface(s.surface) { s.surface = nullptr;  }
		~Surface(void);
		Surface& operator=(Surface& s) { surface = s.surface; s.surface = nullptr; return *this; }

		int width(void) const { return surface->w; }
		int height(void) const { return surface->h; }
		SDL_Surface* get_surface(void) const { return surface; }

		// Accesses individual pixels for 32-bit surfaces.
		const uint32_t& operator ()(int x, int y) const;
		uint32_t& operator ()(int x, int y);

		// Accesses individual pixels for all supported surfaces, converting color value to uint32_t.
		uint32_t get_pixel(int x, int y) const;

		// Returns raw color value for this surface for a given color.
		inline uint32_t color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff) const { return SDL_MapRGBA(surface->format, r, g, b, a); }
		inline uint32_t color(const SDL_Color& color) const { return SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a); }
		// Gets a color for a pixel.
		inline SDL_Color get_color_at(int x, int y) const { return map_color(surface->format, get_pixel(x, y)); }
		
		// Fills surface with color
		void fill(uint32_t color);
		// Replaces a specific color with another.
		void replace(uint32_t src_color, uint32_t dest_color);
		// Returns pixel storage format and data type for OpenGL.
		void query_pixel_format(GLenum & format, GLenum & type) const;
		// Converts this surface to the specific pixel format.
		void convert_format(uint32_t format);

		// Blends another surface on top of this one.
		void blend(const Surface& another);
		void blend(const Surface& another, const Rect& src_rect, const Rect& dest_rect);
		void blend(const Surface& another, int x, int y);
		// Blends flipped surface onto another surface
		void blend_flip_h(const Surface& another, const Rect& src, const Rect& dest);
		void blend_flip_v(const Surface& another, const Rect& src, const Rect& dest);

		// Flips surface 
		void flip_vertical(void) const;
		void flip_horizontal(void) const;
		
		// Applies a transformation to each pixel of the surface. Only supports 32 bit surfaces.
		void apply(const std::function<void(int x, int y, SDL_Surface* s, uint32_t& p)>& f);
	};

	// Creates a new surface for an image file.
	std::unique_ptr<Surface> load_surface(const std::string& filename);
	void save_surface(const Surface&, const std::string& filename);
}
