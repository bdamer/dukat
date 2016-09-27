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

	std::unique_ptr<Texture> TextureCache::load(const std::string& filename)
	{
		auto fqn = resource_dir + "/" + filename;
		logger << "Loading texture [" << filename << "]: ";
	
		auto surface = Surface::from_file(fqn);
		logger << "Created " << surface->get_width() << " by " << surface->get_height() << " surface." << std::endl;

		GLenum format = 0;
		GLenum type = 0;
		switch (surface->get_surface()->format->format)
		{
		// 24 bit
		case SDL_PIXELFORMAT_BGR24:
		case SDL_PIXELFORMAT_BGR888:
			logger << "Unexpected 24 bit pixel format, attempting to convert to RGB888." << std::endl;
			surface->convert_format(SDL_PIXELFORMAT_RGB888);
		case SDL_PIXELFORMAT_RGB24:
		case SDL_PIXELFORMAT_RGB888:
			format = GL_RGB;
			type = GL_UNSIGNED_BYTE;
			break;

		// 32 bit
		case SDL_PIXELFORMAT_ARGB8888:
			logger << "Unexpected 32 bit pixel format, attempting to convert to RGB888." << std::endl;
			surface->convert_format(SDL_PIXELFORMAT_RGBA8888);
		case SDL_PIXELFORMAT_ABGR8888:
			// ABGR seems to convert to RGBA without issue - not sure why
		case SDL_PIXELFORMAT_RGBA8888:
			format = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
			break;

		default:
			logger << "Unsupported format: 0x" << std::hex << surface->get_surface()->format << std::dec << std::endl;
			break;
		}

		if (flip)
		{
			// Flip image from SDL to OpenGL orientation
			surface->flip_horizontal();
		}

		auto res = std::make_unique<Texture>(surface->get_width(), surface->get_height());
		glGenTextures(1, &res->id);
		glBindTexture(GL_TEXTURE_2D, res->id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, res->w, res->h, 0, format, type, surface->get_surface()->pixels);

		// Cleanup
		glBindTexture(GL_TEXTURE_2D, 0);

		return res;
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