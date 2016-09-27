#pragma once

namespace dukat
{
	class Vector2;

	/**
	 * Wrapper around an SDL surface with additional methods for drawing.
	 */
	// TODO: review - most of the functions only work for RGBA surfaces!
	class Surface
	{
	private:
		SDL_Surface* surface;

	public:
		Surface(int width, int height, Uint32 format);
		Surface(SDL_Surface* surface) : surface(surface) { }
		Surface(Surface& s) : surface(s.surface) { s.surface = nullptr;  }
		~Surface(void);
		Surface& operator=(Surface& s) { surface = s.surface; s.surface = nullptr; return *this; }

		SDL_Surface* get_surface(void) const { return surface; }
		void set_surface(SDL_Surface* surface) { this->surface = surface; }
		int get_width(void) const { return surface->w; }
		int get_height(void) const { return surface->h; }
		// Converts this surface to the specific pixel format.
		void convert_format(Uint32 format);
		// Flips surface 
		void flip_horizontal(void);
		void flip_vertical(void);

		inline void fill(Uint32 color) { SDL_FillRect(surface, nullptr, color); }
		inline void set_pixel_fast(int x, int y, Uint32 color) { ((Uint32*)surface->pixels)[y * surface->w + x] = color; }
		inline void set_pixel_fast(int x, int y, const SDL_Color& color) { ((Uint32*)surface->pixels)[y * surface->w + x] = SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a); }
		inline void set_pixel(int x, int y, Uint32 color) { if (y < 0 || y >= surface->h || x < 0 || x >= surface->w) return; ((Uint32*)surface->pixels)[y * surface->w + x] = color; }
		inline void set_pixel(int x, int y, const SDL_Color& color) { if (y < 0 || y >= surface->h || x < 0 || x >= surface->w) return; ((Uint32*)surface->pixels)[y * surface->w + x] = SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a); }
		Uint32 get_pixel(int x, int y) const;

		inline Uint32 color(Uint8 r, Uint8 g, Uint8 b, Uint8 a=0xff) const { return SDL_MapRGBA(surface->format, r, g, b, a); }
		inline Uint32 color(const SDL_Color& color) const { return SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a); }
		SDL_Color get_color_at(int x, int y) const;

		void draw_line(int x0, int y0, int x1, int y1, Uint32 color);
		void draw_circle(int x0, int y0, int radius, Uint32 color);
		void draw_rect(int x, int y, int width, int height, Uint32 color);

		// Blends another surface on top of this one
		void blend(Surface& another);

		static std::unique_ptr<Surface> from_file(const std::string& filename);
	};
}