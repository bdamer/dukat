#include "stdafx.h"
#include <dukat/surface.h>
#include <dukat/log.h>
#include <dukat/sdlutil.h>

namespace dukat
{

	#pragma region Utility Functions

	// Struct for manipulating 24-bit surfaces
	struct Uint24 { Uint8 c[3]; };

	SDL_Color SDL_MapColor(SDL_PixelFormat* fmt, Uint32 pixel)
	{
		Uint32 temp;
		SDL_Color result;

		// Get Red component
		temp = pixel & fmt->Rmask;	// Isolate red component
		temp = temp >> fmt->Rshift;	// Shift it down to 8-bit
		temp = temp << fmt->Rloss;	// Expand to a full 8-bit number
		result.r = (Uint8)temp;

		// Get Green component
		temp = pixel & fmt->Gmask;	// Isolate green component
		temp = temp >> fmt->Gshift;	// Shift it down to 8-bit
		temp = temp << fmt->Gloss;	// Expand to a full 8-bit number
		result.g = (Uint8)temp;

		// Get Blue component
		temp = pixel & fmt->Bmask;	// Isolate blue component
		temp = temp >> fmt->Bshift;	// Shift it down to 8-bit
		temp = temp << fmt->Bloss;	// Expand to a full 8-bit number
		result.b = (Uint8)temp;

		// Get Alpha component
		temp = pixel & fmt->Amask;	// Isolate alpha component
		temp = temp >> fmt->Ashift;	// Shift it down to 8-bit 
		temp = temp << fmt->Aloss;	// Expand to a full 8-bit number 
		result.a = (Uint8)temp;
		return result;
	}

	void flip_surface_v(SDL_Surface* surface)
	{
		const auto size = surface->w * surface->h;
		switch (surface->format->BytesPerPixel)
		{
		case 1:
			dukat::flip_vertical(static_cast<Uint8*>(surface->pixels), static_cast<Uint8*>(surface->pixels) + size, static_cast<size_t>(surface->w));
			break;
		case 2:
			dukat::flip_vertical(static_cast<Uint16*>(surface->pixels), static_cast<Uint16*>(surface->pixels) + size, static_cast<size_t>(surface->w));
			break;
		case 3:
			dukat::flip_vertical(static_cast<Uint24*>(surface->pixels), static_cast<Uint24*>(surface->pixels) + size, static_cast<size_t>(surface->w));
			break;
		case 4:
			dukat::flip_vertical(static_cast<Uint32*>(surface->pixels), static_cast<Uint32*>(surface->pixels) + size, static_cast<size_t>(surface->w));
			break;
		}
	}

	void flip_surface_h(SDL_Surface* surface)
	{
		const auto size = surface->w * surface->h;
		switch (surface->format->BytesPerPixel)
		{
		case 1:
			dukat::flip_horizontal(static_cast<Uint8*>(surface->pixels), static_cast<Uint8*>(surface->pixels) + size, static_cast<size_t>(surface->w));
			break;
		case 2:
			dukat::flip_horizontal(static_cast<Uint16*>(surface->pixels), static_cast<Uint16*>(surface->pixels) + size, static_cast<size_t>(surface->w));
			break;
		case 3:
			dukat::flip_horizontal(static_cast<Uint24*>(surface->pixels), static_cast<Uint24*>(surface->pixels) + size, static_cast<size_t>(surface->w));
			break;
		case 4:
			dukat::flip_horizontal(static_cast<Uint32*>(surface->pixels), static_cast<Uint32*>(surface->pixels) + size, static_cast<size_t>(surface->w));
			break;
		}
	}

	#pragma endregion

	Surface::Surface(int width, int height, Uint32 format)
	{
		int bpp;
		Uint32 rmask, gmask, bmask, amask;
		SDL_PixelFormatEnumToMasks(format, &bpp, &rmask, &gmask, &bmask, &amask);
		surface = SDL_CreateRGBSurface(0, width, height, bpp, rmask, gmask, bmask, amask); 
		if (surface == nullptr)
			throw std::runtime_error("Could not create RGB surface.");
	}

	Surface::~Surface(void)
	{
		if (surface != nullptr)
		{
			SDL_FreeSurface(surface);
			surface = nullptr;
		}
	}

	void Surface::query_pixel_format(GLenum& format, GLenum& type) const
	{
		switch (surface->format->format)
		{
#ifdef OPENGL_CORE
		// 24 bit
		case SDL_PIXELFORMAT_BGR24:
		case SDL_PIXELFORMAT_BGR888:
			format = GL_BGR;
			type = GL_UNSIGNED_BYTE;
		case SDL_PIXELFORMAT_RGB24:
		case SDL_PIXELFORMAT_RGB888:
			format = GL_RGB;
			type = GL_UNSIGNED_BYTE;
			break;
		// 32 bit
		case SDL_PIXELFORMAT_ABGR8888:
			format = GL_RGBA;
			type = surface->format->Rshift == 0xff ? GL_UNSIGNED_INT_8_8_8_8 : GL_UNSIGNED_INT_8_8_8_8_REV;
			break;
		case SDL_PIXELFORMAT_RGBA8888:
			format = GL_RGBA;
			type = surface->format->Rshift == 0xff ? GL_UNSIGNED_INT_8_8_8_8_REV : GL_UNSIGNED_INT_8_8_8_8;
			break;
		case SDL_PIXELFORMAT_BGRA8888:
			format = GL_BGRA;
			type = surface->format->Bshift == 0xff ? GL_UNSIGNED_INT_8_8_8_8_REV : GL_UNSIGNED_INT_8_8_8_8;
			break;
#else
		// 32 bit
		case SDL_PIXELFORMAT_ABGR8888:
			format = GL_RGBA;
			type = surface->format->Rshift == 0xff ? GL_UNSIGNED_BYTE : GL_UNSIGNED_BYTE;
			break;
		case SDL_PIXELFORMAT_RGBA8888:
			format = GL_RGBA;
			type = surface->format->Rshift == 0xff ? GL_UNSIGNED_BYTE : GL_UNSIGNED_BYTE;
			break;
#endif
		default:
			log->warn("Unsupported pixel format: {}", SDL_GetPixelFormatName(surface->format->format));
			format = 0;
			type = 0;
			break;
		}
	}

	Uint32 Surface::get_pixel(int x, int y) const
	{
		// Here p is the address to the pixel we want to retrieve
		Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
		switch (surface->format->BytesPerPixel) 
		{
		case 1:
			return *p;
		case 2:
			return *(Uint16 *)p;
		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
				return p[0] << 16 | p[1] << 8 | p[2];
			else
				return p[0] | p[1] << 8 | p[2] << 16;
		case 4:
			return *(Uint32 *)p;
		default:
			return 0;       // shouldn't happen, but avoids warnings
		}
	}

	void Surface::draw_line(int x0, int y0, int x1, int y1, Uint32 color)
	{
		// check if slope is steep
		bool steep = abs(y1 - y0) > abs(x1 - x0);
		if (steep) 
		{
			std::swap(x0, y0);
			std::swap(x1, y1);
		}
		// check if line is drawn from right-to-left
		if (x0 > x1)
		{
			std::swap(x0, x1);
			std::swap(y0, y1);
		}

		int deltax = x1 - x0;
		int deltay = abs(y1 - y0);
		int error = deltax / 2;
		int ystep;
		int y = y0;
		if (y0 < y1)
			ystep = 1;
		else
			ystep = -1;
		for (int x = x0; x < x1; x++)
		{
			if (x < 0 || x >= surface->w)
				continue;
			if (y < 0 || y >= surface->h)
				continue;

			if (steep)
				set_pixel(y, x, color);
			else
				set_pixel(x, y, color);
			error = error - deltay;
			if (error < 0)
			{
				y = y + ystep;
				error = error + deltax;
			}
		}
	}

	void Surface::draw_circle(int x0, int y0, int radius, Uint32 color)
	{
		int x = radius, y = 0;
		int radiusError = 1 - x;
		while (x >= y)
		{
			set_pixel(x + x0, y + y0, color);
			set_pixel(y + x0, x + y0, color);
			set_pixel(-x + x0, y + y0, color);
			set_pixel(-y + x0, x + y0, color);
			set_pixel(-x + x0, -y + y0, color);
			set_pixel(-y + x0, -x + y0, color);
			set_pixel(x + x0, -y + y0, color);
			set_pixel(y + x0, -x + y0, color);
			y++;

			if (radiusError < 0)
			{
				radiusError += 2 * y + 1;
			}
			else
			{
				x--;
				radiusError += 2 * (y - x + 1);
			}
		}
	}

	void Surface::draw_rect(int x, int y, int width, int height, Uint32 color)
	{
		draw_line(x, y, x + width, y, color);
		draw_line(x + width, y, x + width, y + height, color);
		draw_line(x + width, y + height, x, y + height, color);
		draw_line(x, y + height, x, y, color);
	}

	void Surface::fill_circle(int x0, int y0, int radius, Uint32 color)
	{
		int x = radius, y = 0;
		int radiusError = 1 - x;
		while (x >= y)
		{
			draw_line(x0 - x, y + y0, x0 + x, y + y0, color);
			draw_line(x0 - y, x + y0, x0 + y, x + y0, color);
			draw_line(x0 - x, y0 - y, x0 + x, y0 - y, color);
			draw_line(x0 - y, y0 - x, x0 + y, y0 - x, color);
			y++;

			if (radiusError < 0)
			{
				radiusError += 2 * y + 1;
			}
			else
			{
				x--;
				radiusError += 2 * (y - x + 1);
			}
		}
	}

	void Surface::fill_rect(int x, int y, int width, int height, Uint32 color)
	{
		SDL_Rect rect = { x, y, width, height };
		SDL_FillRect(surface, &rect, color);
	}

	void Surface::fill(Uint32 color)
	{
		SDL_FillRect(surface, nullptr, color);
	}

	void Surface::convert_format(Uint32 format)
	{
		SDL_Surface* tmp = SDL_ConvertSurfaceFormat(surface, format, 0);
		if (tmp == nullptr)
		{
			std::stringstream ss;
			ss << "Could not convert surface: " << SDL_GetError();
			throw std::runtime_error(ss.str());
		}
		SDL_FreeSurface(surface);
		surface = tmp;
	}

	void Surface::blend(const Surface& another)
	{
		SDL_BlitSurface(another.get_surface(), nullptr, surface, nullptr);
	}

	void Surface::blend(const Surface& another, const Rect& src, const Rect& dest)
	{
		SDL_BlitSurface(another.get_surface(), (SDL_Rect*)&src, surface, (SDL_Rect*)&dest);
	}

	void Surface::blend(const Surface& another, int x, int y)
	{
		SDL_Rect r{ x, y, another.get_width(), another.get_height() };
		SDL_BlitSurface(another.get_surface(), nullptr, surface, &r);
	}

	void Surface::blend_flip_h(const Surface& another, const Rect& src, const Rect& dest)
	{
		auto tmp_surface = SDL_CreateRGBSurface(0, src.w, src.h, surface->format->BitsPerPixel, 
			surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
		if (tmp_surface == nullptr)
			throw std::runtime_error("Failed to create surface.");
		SDL_BlitSurface(surface, (SDL_Rect*)&src, tmp_surface, nullptr);
		flip_surface_h(tmp_surface);
		SDL_BlitSurface(tmp_surface, nullptr, another.surface, (SDL_Rect*)&dest);
		SDL_FreeSurface(tmp_surface);
	}

	void Surface::blend_flip_v(const Surface& another, const Rect& src, const Rect& dest)
	{
		auto tmp_surface = SDL_CreateRGBSurface(0, src.w, src.h, surface->format->BitsPerPixel,
			surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
		if (tmp_surface == nullptr)
			throw std::runtime_error("Failed to create surface.");
		SDL_BlitSurface(surface, (SDL_Rect*)&src, tmp_surface, nullptr);
		flip_surface_v(tmp_surface);
		SDL_BlitSurface(tmp_surface, nullptr, another.surface, (SDL_Rect*)&dest);
		SDL_FreeSurface(tmp_surface);
	}

	std::unique_ptr<Surface> Surface::from_file(const std::string& filename)
	{
		auto surface = IMG_Load(filename.c_str());
		if (surface == nullptr)
		{
			std::stringstream ss;
			ss << "Could not load surface: " << filename;
			throw std::runtime_error(ss.str());
		}
		return std::make_unique<Surface>(surface);
	}

	void Surface::save_to_file(const std::string& filename) const
	{
		auto res = IMG_SavePNG(surface, filename.c_str());
		sdl_check_result(res, "Save surface");
	}

	void Surface::apply(const std::function<void(int x, int y, SDL_Surface* s, uint32_t* p)>& f)
	{
		auto ptr = static_cast<uint32_t*>(surface->pixels);
		for (auto y = 0; y < surface->h; y++)
		{
			for (auto x = 0; x < surface->w; x++)
			{
				f(x, y, surface, ptr++);
			}
		}
	}	
}