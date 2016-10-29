#include "stdafx.h"
#include "log.h"
#include "surface.h"
#include "texturecache.h"
#include "dds.h"
#include "sysutil.h"

namespace dukat
{
	Texture::Texture(const Surface& surface) : id(0)
	{
		w = surface.get_width();
		h = surface.get_height();
		load_data(surface);
	}

	void Texture::load_data(const Surface& surface)
	{
		GLenum format = 0, type = 0;
		surface.query_pixel_format(format, type);

		if (id == 0)
		{
			glGenTextures(1, &id);
		}
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, format, type, surface.get_surface()->pixels);
#ifdef _DEBUG
		glBindTexture(GL_TEXTURE_2D, 0);
#endif
	}

	TextureCache::TextureCache(const std::string& resource_dir, bool flip) : resource_dir(resource_dir), flip(flip)
	{
	}

	TextureCache::~TextureCache(void)
	{
		free_all();
	}

	Surface* TextureCache::load_surface(const std::string& filename)
	{
		if (surfaces.count(filename) == 0)
		{
			auto fqn = resource_dir + "/" + filename;
			auto surface = Surface::from_file(fqn);
			// Perform necessary conversion
			switch (surface->get_surface()->format->format)
			{
				// 24 bit
			case SDL_PIXELFORMAT_BGR24:
			case SDL_PIXELFORMAT_BGR888:
				logger << "Unexpected 24 bit pixel format, attempting to convert to RGB888." << std::endl;
				surface->convert_format(SDL_PIXELFORMAT_RGB888);
				// 32 bit
			case SDL_PIXELFORMAT_ARGB8888:
				logger << "Unexpected 32 bit pixel format, attempting to convert to RGB888." << std::endl;
				surface->convert_format(SDL_PIXELFORMAT_RGBA8888);
			}
			if (flip)
			{
				// Flip image from SDL to OpenGL orientation
				surface->flip_horizontal();
			}
			surfaces[filename] = std::move(surface);
		}
		return surfaces[filename].get();
	}

	std::unique_ptr<Texture> TextureCache::load(const std::string& filename)
	{
		logger << "Loading texture [" << filename << "]: ";
		auto surface = load_surface(filename);
		logger << "Created " << surface->get_width() << " by " << surface->get_height() << " surface." << std::endl;
		return std::make_unique<Texture>(*surface);
	}

	Texture* TextureCache::get(const std::string& filename)
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
				textures[filename] = load(filename);
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