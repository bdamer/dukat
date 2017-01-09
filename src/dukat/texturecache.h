#pragma once

#include <unordered_map>
#include "surface.h"

namespace dukat
{
	typedef GLuint TextureId;

	enum TextureFilterProfile
	{
		ProfileNearest,
		ProfileLinear,
		ProfileMipMapped,
		ProfileAnisotropic
	};

	struct Texture
	{
		TextureId id;
		int w, h;

		Texture(void) : id(0), w(0), h(0) { }
		Texture(int w, int h) : id(0), w(w), h(h) { }
		Texture(const Surface& surface, TextureFilterProfile profile = ProfileNearest);
		~Texture(void) { glDeleteTextures(1, &id); }
	
		void load_data(const Surface& surface, TextureFilterProfile profile = ProfileNearest);
	};

	class TextureCache
	{
	private:
		const std::string resource_dir;
		const bool flip;
		std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
		std::unordered_map<std::string, std::unique_ptr<Surface>> surfaces;
		std::unique_ptr<Texture> load(const std::string& filename, TextureFilterProfile profile);

	public:
		// Creates a new texture cache. Set flip to true to change images from 
		// natural orientation (y-axis points down) to OpenGL standard (y-axis points up). 
		TextureCache(const std::string& resource_dir, bool flip = false);
		~TextureCache(void);

		// Returns a texture for a image file.
		Texture* get(const std::string& filename, TextureFilterProfile profile = ProfileNearest);
		// Returns a texture with a specific id if it exists in the cache.
		Texture* get(const TextureId id) const;
		// Puts texture entry into cache.
		void put(const std::string& name, std::unique_ptr<Texture> texture) { textures.insert( std::make_pair(name, std::move(texture))); }
		// Helper method to load a surface from a file.
		Surface* load_surface(const std::string& filename);

		// Frees all cached textures.
		void free_all(void);
		// Frees the texture corresponding to the image file.
		void free(const std::string& filename);
	};
}
