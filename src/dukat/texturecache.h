#pragma once

#include <unordered_map>

namespace dukat
{
	typedef GLuint TextureId;

	struct Texture
	{
		TextureId id;
		int w, h;

		Texture(void) : id(0), w(0), h(0) { }
		Texture(int w, int h) : id(0), w(w), h(h) { }
		~Texture(void) { glDeleteTextures(1, &id); }
	};

	class TextureCache
	{
	private:
		const std::string resource_dir;
		const bool flip;
		std::unordered_map<std::string,std::unique_ptr<Texture>> textures;
		std::unique_ptr<Texture> load(const std::string& filename);

	public:
		// Creates a new texture cache. Set flip to true to change images from 
		// natural orientation (y-axis points down) to OpenGL standard (y-axis points up). 
		TextureCache(const std::string& resource_dir, bool flip = false);
		~TextureCache(void);

		// Returns a texture for a image file.
		Texture* get(const std::string& filename);
		// Returns a texture with a specific id if it exists in the cache.
		Texture* get(const TextureId id) const;
		// Puts texture entry into cache.
		void put(const std::string& name, std::unique_ptr<Texture> texture) { textures.insert( std::make_pair(name, std::move(texture))); }

		// Frees all cached textures.
		void free_all(void);
		// Frees the texture corresponding to the image file.
		void free(const std::string& filename);
	};
}
