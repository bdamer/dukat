#include "stdafx.h"
#include "surface.h"

namespace dukat
{
	Surface::Surface(int width, int height, Uint32 format)
	{
		int bpp;
		Uint32 rmask, gmask, bmask, amask;
		SDL_PixelFormatEnumToMasks(format, &bpp, &rmask, &gmask, &bmask, &amask);
		surface = SDL_CreateRGBSurface(0, width, height, bpp, rmask, gmask, bmask, amask); 
		if (surface == nullptr)
		{
			throw std::runtime_error("Could not create RGB surface.");
		}
	}

	Surface::~Surface(void)
	{
		if (surface != nullptr)
		{
			SDL_FreeSurface(surface);
			surface = nullptr;
		}
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

	void Surface::flip_horizontal(void)
	{
		Uint8* top = (Uint8*)surface->pixels;
		Uint8* bottom = ((Uint8*)surface->pixels) + surface->pitch * (surface->h - 1);
		while (top < bottom)
		{
			for (int i = 0; i < surface->pitch; i++)
			{
				Uint8 tmp = *top;
				*top++ = *bottom;
				*bottom++ = tmp;
			}
			bottom -= 2 * surface->pitch;
		}
	}

	void Surface::flip_vertical(void)
	{
		Uint8 bpp = surface->format->BytesPerPixel;
		for (int row = 0; row < surface->h; row++)
		{
			Uint8* left = (Uint8*)surface->pixels + row * surface->pitch;
			Uint8* right = left + surface->pitch - bpp;
			while (left < right)
			{
				for (int i = 0; i < bpp; i++)
				{
					Uint8 tmp = left[i];
					left[i] = right[i];
					right[i] = tmp;
				}
				left += bpp;
				right -= bpp;
			}
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

	SDL_Color Surface::get_color_at(int x, int y) const
	{
		Uint32 temp;
		SDL_Color result;
		SDL_PixelFormat *fmt = surface->format;
		Uint32 pixel = get_pixel(x, y);

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

	void Surface::blend(Surface& another)
	{
		SDL_BlitSurface(another.get_surface(), nullptr, surface, nullptr);
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
}