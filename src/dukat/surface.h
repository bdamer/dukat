#pragma once

#include "color.h"
#include "rect.h"

namespace dukat
{
	class Vector2;

	/**
	 * Wrapper for a 32-bit SDL surface.
	 */
	class Surface
	{
	private:
		SDL_Surface* surface;

		inline void set_pixel_fast(int x, int y, Uint32 color) { ((Uint32*)surface->pixels)[y * surface->w + x] = color; }
		inline void set_pixel_fast(int x, int y, const SDL_Color& color) { ((Uint32*)surface->pixels)[y * surface->w + x] = SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a); }

	public:
		Surface(int width, int height, Uint32 format);
		Surface(SDL_Surface* surface) : surface(surface) { }
		Surface(Surface& s) : surface(s.surface) { s.surface = nullptr;  }
		~Surface(void);
		Surface& operator=(Surface& s) { surface = s.surface; s.surface = nullptr; return *this; }

		// Creates a new surface for an image file.
		static std::unique_ptr<Surface> from_file(const std::string& filename);
		void save_to_file(const std::string& filename) const;

		SDL_Surface* get_surface(void) const { return surface; }
		void set_surface(SDL_Surface* surface) { this->surface = surface; }
		int get_width(void) const { return surface->w; }
		int get_height(void) const { return surface->h; }
		// Returns pixel storage format and data type for OpenGL.
		void query_pixel_format(GLenum& format, GLenum& type) const;

		// Sets a pixel to a color.		
		inline void set_pixel(int x, int y, Uint32 color) { if (y < 0 || y >= surface->h || x < 0 || x >= surface->w) return; ((Uint32*)surface->pixels)[y * surface->w + x] = color; }
		inline void set_pixel(int x, int y, const SDL_Color& color) { if (y < 0 || y >= surface->h || x < 0 || x >= surface->w) return; ((Uint32*)surface->pixels)[y * surface->w + x] = SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a); }
		inline void set_pixel(int x, int y, const Color& color) { if (y < 0 || y >= surface->h || x < 0 || x >= surface->w) return; ((Uint32*)surface->pixels)[y * surface->w + x] = SDL_MapRGBA(surface->format, (Uint8)(255.0f * color.r), (Uint8)(255.0f * color.g), (Uint8)(255.0f * color.b), (Uint8)(255.0f * color.a)); }

		// Gets a color for a pixel.
		Uint32 get_pixel(int x, int y) const;
		SDL_Color get_color_at(int x, int y) const;
		inline Uint32 color(Uint8 r, Uint8 g, Uint8 b, Uint8 a=0xff) const { return SDL_MapRGBA(surface->format, r, g, b, a); }
		inline Uint32 color(const SDL_Color& color) const { return SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a); }

		// Converts this surface to the specific pixel format.
		void convert_format(Uint32 format);
		// Blends another surface on top of this one
		void blend(const Surface& another);
		void blend(const Surface& another, const Rect& src, const Rect& dest);
		// Flips surface 
		void flip_horizontal(void);
		void flip_vertical(void);
		// Drawing methods
		void draw_line(int x0, int y0, int x1, int y1, Uint32 color);
		void draw_circle(int x0, int y0, int radius, Uint32 color);
		void draw_rect(int x, int y, int width, int height, Uint32 color);
		void fill_circle(int x0, int y0, int radius, Uint32 color);
		void fill_rect(int x, int y, int width, int height, Uint32 color);
		void fill(Uint32 color);
	};
}