#include "stdafx.h"
#include <dukat/surface.h>
#include <dukat/bit.h>
#include <dukat/log.h>
#include <dukat/sdlutil.h>

namespace dukat
{
	// Struct for manipulating 24-bit surfaces
	struct Uint24 { Uint8 c[3]; };

	Surface::Surface(int width, int height, uint32_t format)
	{
		int bpp;
		uint32_t rmask, gmask, bmask, amask;
		SDL_PixelFormatEnumToMasks(format, &bpp, &rmask, &gmask, &bmask, &amask);
		surface = SDL_CreateRGBSurface(0, width, height, bpp, rmask, gmask, bmask, amask); 
		if (surface == nullptr)
			throw std::runtime_error("Could not create RGB surface.");
	}

	Surface::Surface(SDL_Surface* surface) : surface(surface)
	{
		if (surface == nullptr)
			throw std::runtime_error("Failed to create surface for invalid SDL_Surface.");
	}

	Surface::~Surface(void)
	{
		if (surface != nullptr)
		{
			SDL_FreeSurface(surface);
			surface = nullptr;
		}
	}

	Color Surface::get_color(int x, int y) const
	{
		const auto raw = get_pixel(x, y);
		const auto color = map_raw(raw);
		return color_rgba(color.r, color.g, color.b, color.a);
	}

	void Surface::set_pixel(int x, int y, uint32_t p)
	{
		assert(x >= 0 && x < surface->w && y >= 0 && y < surface->h);
		switch (surface->format->BytesPerPixel)
		{
		case 1:
			write(x, y, static_cast<uint8_t>(p));
			break;
		case 2:
			write(x, y, static_cast<uint16_t>(p));
			break;
		case 3:
			*reinterpret_cast<Uint24*>(raw_ptr(x, y)) 
				= Uint24{ static_cast<uint8_t>(p >> 24), static_cast<uint8_t>(p >> 16), static_cast<uint8_t>(p >> 8) };
			break;
		case 4:
			write(x, y, p);
			break;
		}
	}

	uint32_t Surface::get_pixel(int x, int y) const
	{
		assert(x >= 0 && x < surface->w&& y >= 0 && y < surface->h);
		auto p = raw_ptr(x, y);
		switch (surface->format->BytesPerPixel) 
		{
		case 1:
			return *p;
		case 2:
			return *reinterpret_cast<uint16_t*>(p);
		case 3:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
				return p[0] << 16 | p[1] << 8 | p[2];
			else
				return p[0] | p[1] << 8 | p[2] << 16;
		case 4:
			return *reinterpret_cast<uint32_t*>(p);
		default:
			return 0; // shouldn't happen, but avoids warnings
		}
	}

	void Surface::read(int x, int y, uint32_t& c) const
	{
		assert(surface->format->BytesPerPixel == 4);
		c = *reinterpret_cast<uint32_t*>(raw_ptr(x, y));
	}

	void Surface::write(int x, int y, uint32_t c)
	{
		assert(surface->format->BytesPerPixel == 4);
		*reinterpret_cast<uint32_t*>(raw_ptr(x, y)) = c;
	}

	void Surface::read(int x, int y, uint16_t& c) const
	{
		assert(surface->format->BytesPerPixel == 2);
		c = *reinterpret_cast<uint16_t*>(raw_ptr(x, y));
	}

	void Surface::write(int x, int y, uint16_t c)
	{
		assert(surface->format->BytesPerPixel == 2);
		*reinterpret_cast<uint16_t*>(raw_ptr(x, y)) = c;
	}

	void Surface::read(int x, int y, uint8_t& c) const
	{
		assert(surface->format->BytesPerPixel == 1);
		c = *raw_ptr(x, y);
	}

	void Surface::write(int x, int y, uint8_t c)
	{
		assert(surface->format->BytesPerPixel == 1);
		*raw_ptr(x, y) = c;
	}

	SDL_Color Surface::map_raw(uint32_t pixel) const
	{
		uint32_t temp;
		SDL_Color result;

		auto fmt = surface->format;
		if (fmt->BytesPerPixel == 1)
		{
			return fmt->palette->colors[static_cast<uint8_t>(pixel)];
		}
		else // TRUE color
		{
			// Get Red component
			temp = pixel & fmt->Rmask;	// Isolate red component
			temp = temp >> fmt->Rshift;	// Shift it down to 8-bit
			temp = temp << fmt->Rloss;	// Expand to a full 8-bit number
			result.r = static_cast<uint8_t>(temp);

			// Get Green component
			temp = pixel & fmt->Gmask;	// Isolate green component
			temp = temp >> fmt->Gshift;	// Shift it down to 8-bit
			temp = temp << fmt->Gloss;	// Expand to a full 8-bit number
			result.g = static_cast<uint8_t>(temp);

			// Get Blue component
			temp = pixel & fmt->Bmask;	// Isolate blue component
			temp = temp >> fmt->Bshift;	// Shift it down to 8-bit
			temp = temp << fmt->Bloss;	// Expand to a full 8-bit number
			result.b = static_cast<uint8_t>(temp);

			// Get Alpha component
			temp = pixel & fmt->Amask;	// Isolate alpha component
			temp = temp >> fmt->Ashift;	// Shift it down to 8-bit 
			temp = temp << fmt->Aloss;	// Expand to a full 8-bit number 
			result.a = static_cast<uint8_t>(temp);
		}

		return result;
	}

	uint32_t Surface::raw_color(const Color& color) const
	{
		const auto c = sdl_color(color);
		return SDL_MapRGBA(surface->format, c.r, c.g, c.b, c.a);
	}

	void Surface::replace(const Color& src_color, const Color& dest_color)
	{
		const auto sc = raw_color(src_color);
		const auto dc = raw_color(src_color);
		auto ptr = static_cast<uint32_t*>(surface->pixels);
		auto end = ptr + surface->w * surface->h;
		while (ptr < end)
		{
			if (*ptr == sc)
				*ptr = dc;
			++ptr;
		}
	}

	void Surface::replace(const std::vector<Color>& src_colors, const std::vector<Color>& dest_colors)
	{
		assert(src_colors.size() == dest_colors.size());

		std::unordered_map<uint32_t, uint32_t> colors;
		for (auto i = 0u; i < src_colors.size(); i++)
			colors.insert(std::make_pair(raw_color(src_colors[i]), raw_color(dest_colors[i])));

		auto ptr = static_cast<uint32_t*>(surface->pixels);
		auto end = ptr + surface->w * surface->h;
		while (ptr < end)
		{
			if (colors.count(*ptr))
				*ptr = colors[*ptr];
			++ptr;
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

	void Surface::convert_format(uint32_t format)
	{
		auto tmp = SDL_ConvertSurfaceFormat(surface, format, 0);
		if (tmp == nullptr)
		{
			std::stringstream ss;
			ss << "Could not convert surface: " << SDL_GetError();
			throw std::runtime_error(ss.str());
		}
		SDL_FreeSurface(surface);
		surface = tmp;
	}

	// Note: rows must be swapped using surface->pitch (bytes), not w * BytesPerPixel -
	// SDL pads each row to a 4-byte boundary, so the two only match when w * BytesPerPixel
	// is itself a multiple of 4.
	void Surface::flip_vertical(void)
	{
		auto pixels = static_cast<uint8_t*>(surface->pixels);
		const auto pitch = static_cast<size_t>(surface->pitch);
		std::vector<uint8_t> row(pitch);
		for (auto y = 0; y < surface->h / 2; y++)
		{
			auto top = pixels + y * pitch;
			auto bottom = pixels + (surface->h - 1 - y) * pitch;
			std::memcpy(row.data(), top, pitch);
			std::memcpy(top, bottom, pitch);
			std::memcpy(bottom, row.data(), pitch);
		}
	}

	void Surface::flip_horizontal(void)
	{
		auto pixels = static_cast<uint8_t*>(surface->pixels);
		const auto pitch = static_cast<size_t>(surface->pitch);
		const auto bpp = static_cast<size_t>(surface->format->BytesPerPixel);
		std::vector<uint8_t> tmp(bpp);
		for (auto y = 0; y < surface->h; y++)
		{
			auto left = pixels + y * pitch;
			auto right = left + (surface->w - 1) * bpp;
			while (left < right)
			{
				std::memcpy(tmp.data(), left, bpp);
				std::memcpy(left, right, bpp);
				std::memcpy(right, tmp.data(), bpp);
				left += bpp;
				right -= bpp;
			}
		}
	}

	void Surface::apply(const std::function<void(int x, int y, SDL_Surface* s, uint32_t& p)>& f)
	{
		auto ptr = static_cast<uint32_t*>(surface->pixels);
		for (auto y = 0; y < surface->h; y++)
		{
			for (auto x = 0; x < surface->w; x++)
			{
				f(x, y, surface, *(ptr++));
			}
		}
	}

#pragma region Utility Functions

	std::unique_ptr<Surface> load_surface(const std::string& filename)
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

	void save_surface(const Surface& surface, const std::string& filename)
	{
		auto res = IMG_SavePNG(surface.get_surface(), filename.c_str());
		sdl_check_result(res, "Save surface");
	}

#pragma endregion

}