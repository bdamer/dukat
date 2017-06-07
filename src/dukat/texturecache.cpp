#include "stdafx.h"
#include "log.h"
#include "surface.h"
#include "texturecache.h"
#include "dds.h"
#include "sysutil.h"

namespace dukat
{
	TextureCache::TextureCache(const std::string& resource_dir, bool flip) : resource_dir(resource_dir), flip(flip)
	{
	}

	TextureCache::~TextureCache(void)
	{
		free_all();
	}

	Surface* TextureCache::load_surface(const std::string& filename, bool hflip, bool vflip)
	{
		if (surfaces.count(filename) == 0)
		{
			auto fqn = resource_dir + "/" + filename;
			auto surface = Surface::from_file(fqn);
			// Perform necessary conversion
 			switch (surface->get_surface()->format->format)
			{
				// 8 bit
			//case SDL_PIXELFORMAT_INDEX8:
			//	logger << "Unexpected 8 bit pixel format, attempting to convert to RGB888." << std::endl;
			//	surface->convert_format(SDL_PIXELFORMAT_RGB888);
			//	break;
				// 24 bit
			case SDL_PIXELFORMAT_BGR24:
			case SDL_PIXELFORMAT_BGR888:
				logger << "Unexpected 24 bit pixel format, attempting to convert to RGB888." << std::endl;
				surface->convert_format(SDL_PIXELFORMAT_RGB888);
				break;
				// 32 bit
			case SDL_PIXELFORMAT_ARGB8888:
				logger << "Unexpected 32 bit pixel format, attempting to convert to RGB888." << std::endl;
				surface->convert_format(SDL_PIXELFORMAT_RGBA8888);
				break;
			}
			if (hflip)
			{
				// Flip image from SDL to OpenGL orientation
				surface->flip_horizontal();
			}
			if (vflip)
			{
				surface->flip_vertical();
			}
			surfaces[filename] = std::move(surface);
		}
		return surfaces[filename].get();
	}

	std::unique_ptr<Texture> TextureCache::load(const std::string& filename, TextureFilterProfile profile)
	{
		logger << "Loading texture [" << filename << "]: " << std::endl;
		auto surface = load_surface(filename, flip);
		logger << "Created " << surface->get_width() << "x" << surface->get_height() << " " 
			<< SDL_GetPixelFormatName(surface->get_surface()->format->format) << " surface." << std::endl;
		return std::make_unique<Texture>(*surface, profile);
	}

	Texture* TextureCache::get(const std::string& filename, TextureFilterProfile profile)
	{
		if (textures.count(filename) == 0)
		{
			auto ext = get_extension(filename);
			// TODO: base this on file header not extension (use streams if possible)
			if (ext == "dds" || ext == "tga" || ext == "TGA")
			{
				textures[filename] = load_dds(resource_dir + "/" + filename);
			}
			else
			{
				textures[filename] = load(filename, profile);
			}
		}
		return textures[filename].get();
	}

	Texture* TextureCache::get(const TextureId tid) const
	{
		for (auto& t : textures)
		{
			if (t.second->id == tid)
			{
				return t.second.get();
			}
		}
		throw std::runtime_error("Could not find texture by id!");
	}

	void TextureCache::free(const std::string& filename)
	{
		if (textures.count(filename) != 0)
		{
			textures.erase(filename);
		}
	}

	void TextureCache::free_all(void)
	{
		logger << "Freeing " << textures.size() << " texture(s)..." << std::endl;
		textures.empty();
	}
}