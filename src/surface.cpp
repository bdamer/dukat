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

	SDL_Color Surface::map_color(SDL_PixelFormat* fmt, uint32_t pixel) const
	{
		uint32_t temp;
		SDL_Color result;

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
		return result;
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

	const uint32_t& Surface::operator()(int x, int y) const
	{
		assert(x >= 0 && x < surface->w&& y >= 0 && y < surface->h);
		return static_cast<uint32_t*>(surface->pixels)[y * surface->w + x];
	}

	uint32_t& Surface::operator()(int x, int y)
	{
		assert(x >= 0 && x < surface->w && y >= 0 && y < surface->h);
		return static_cast<uint32_t*>(surface->pixels)[y * surface->w + x];
	}

	uint32_t Surface::get_pixel(int x, int y) const
	{
		// Here p is the address to the pixel we want to retrieve
		auto p = static_cast<uint8_t*>(surface->pixels) + y * surface->pitch + x * surface->format->BytesPerPixel;
		switch (surface->format->BytesPerPixel) 
		{
		case 1:
			return *p;
		case 2:
			return *reinterpret_cast<uint16_t*>(p);
		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
				return p[0] << 16 | p[1] << 8 | p[2];
			else
				return p[0] | p[1] << 8 | p[2] << 16;
		case 4:
			return *reinterpret_cast<uint32_t*>(p);
		default:
			return 0;       // shouldn't happen, but avoids warnings
		}
	}

	void Surface::fill(uint32_t color)
	{
		SDL_FillRect(surface, nullptr, color);
	}

	void Surface::replace(uint32_t src_color, uint32_t dest_color)
	{
		auto ptr = static_cast<uint32_t*>(surface->pixels);
		auto end = ptr + surface->w * surface->h;
		while (ptr < end)
		{
			if (*ptr == src_color)
				*ptr = dest_color;
			++ptr;
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

	void Surface::blend(const Surface& another)
	{
		SDL_BlitSurface(another.surface, nullptr, surface, nullptr);
	}

	void Surface::blend(const Surface& another, const Rect& src, const Rect& dest)
	{
		SDL_BlitSurface(another.surface, (SDL_Rect*)&src, surface, (SDL_Rect*)&dest);
	}

	void Surface::blend(const Surface& another, int x, int y)
	{
		SDL_Rect r{ x, y, another.width(), another.height() };
		SDL_BlitSurface(another.surface, nullptr, surface, &r);
	}

	void Surface::blend_flip_h(const Surface& another, const Rect& src, const Rect& dest)
	{
		Surface tmp_surface(SDL_CreateRGBSurface(0, src.w, src.h, surface->format->BitsPerPixel,
			surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask));
		tmp_surface.blend(*this, src, src);
		tmp_surface.flip_horizontal();
		SDL_BlitSurface(tmp_surface.surface, nullptr, another.surface, (SDL_Rect*)&dest);
	}

	void Surface::blend_flip_v(const Surface& another, const Rect& src, const Rect& dest)
	{
		Surface tmp_surface(SDL_CreateRGBSurface(0, src.w, src.h, surface->format->BitsPerPixel,
			surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask));
		tmp_surface.blend(*this, src, src);
		tmp_surface.flip_vertical();
		SDL_BlitSurface(tmp_surface.surface, nullptr, another.surface, (SDL_Rect*)&dest);
	}

	void Surface::flip_vertical(void) const 
	{
		const auto size = surface->w * surface->h;
		switch (surface->format->BytesPerPixel)
		{
		case 1:
			dukat::flip_vertical(static_cast<uint8_t*>(surface->pixels), static_cast<uint8_t*>(surface->pixels) + size, static_cast<size_t>(surface->w));
			break;
		case 2:
			dukat::flip_vertical(static_cast<uint16_t*>(surface->pixels), static_cast<uint16_t*>(surface->pixels) + size, static_cast<size_t>(surface->w));
			break;
		case 3:
			dukat::flip_vertical(static_cast<Uint24*>(surface->pixels), static_cast<Uint24*>(surface->pixels) + size, static_cast<size_t>(surface->w));
			break;
		case 4:
			dukat::flip_vertical(static_cast<uint32_t*>(surface->pixels), static_cast<uint32_t*>(surface->pixels) + size, static_cast<size_t>(surface->w));
			break;
		}
	}

	void Surface::flip_horizontal(void) const 
	{ 
		const auto size = surface->w * surface->h;
		switch (surface->format->BytesPerPixel)
		{
		case 1:
			dukat::flip_horizontal(static_cast<uint8_t*>(surface->pixels), static_cast<uint8_t*>(surface->pixels) + size, static_cast<size_t>(surface->w));
			break;
		case 2:
			dukat::flip_horizontal(static_cast<uint16_t*>(surface->pixels), static_cast<uint16_t*>(surface->pixels) + size, static_cast<size_t>(surface->w));
			break;
		case 3:
			dukat::flip_horizontal(static_cast<Uint24*>(surface->pixels), static_cast<Uint24*>(surface->pixels) + size, static_cast<size_t>(surface->w));
			break;
		case 4:
			dukat::flip_horizontal(static_cast<uint32_t*>(surface->pixels), static_cast<uint32_t*>(surface->pixels) + size, static_cast<size_t>(surface->w));
			break;
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

	void replace_surface_colors(Surface& surface, const std::vector<SDL_Color>& src_colors, const std::vector<SDL_Color>& dest_colors)
	{
		assert(src_colors.size() == dest_colors.size());

		std::unordered_map<uint32_t, uint32_t> colors;
		for (auto i = 0u; i < src_colors.size(); i++)
			colors.insert(std::make_pair(surface.color(src_colors[i]), surface.color(dest_colors[i])));

		auto sfc = surface.get_surface();
		auto ptr = static_cast<uint32_t*>(sfc->pixels);
		auto end = ptr + sfc->w * sfc->h;
		while (ptr < end)
		{
			if (colors.count(*ptr))
				*ptr = colors[*ptr];
			++ptr;
		}
	}

#pragma endregion
}