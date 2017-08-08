#pragma once

#include <unordered_map>
#include "surface.h"
#include "sysutil.h"
#include "texture.h"

namespace dukat
{
	class TextureCache
	{
	private:
		const std::string resource_dir;
		const bool flip;
		uint32_t last_id;
		std::unordered_map<uint32_t, std::unique_ptr<Texture>> textures;
		std::unordered_map<std::string, std::unique_ptr<Surface>> surfaces;
		std::unique_ptr<Texture> load(const std::string& filename, TextureFilterProfile profile);

	public:
		// Creates a new texture cache. Set flip to true to change images from 
		// natural orientation (y-axis points down) to OpenGL standard (y-axis points up). 
		TextureCache(const std::string& resource_dir, bool flip = false);
		~TextureCache(void);

		// Helper method to load a surface from a file.
		Surface* load_surface(const std::string& filename, bool hflip = false, bool vflip = false);
		// Generates an ID that can be used to store and retrieve dynamic textures and surfaces.
		uint32_t generate_id(void) { return ++last_id; }

		// Returns a texture for a image file.
		Texture* get(const std::string& filename, TextureFilterProfile profile = ProfileNearest);
		Texture* get(uint32_t id);
		// Puts texture entry into cache.
		void put(const std::string& filename, std::unique_ptr<Texture> texture) { put(compute_hash(filename), std::move(texture)); }
		void put(uint32_t id, std::unique_ptr<Texture> texture) { textures.insert(std::make_pair(id, std::move(texture))); }
		// Frees all cached textures.
		void free_all(void);
		// Frees the texture corresponding to the image file.
		void free(const std::string& filename) { free(compute_hash(filename)); }
		void free(uint32_t id);
	};
}
