#pragma once

#include <unordered_map>
#include "surface.h"
#include "string.h"
#include "texture.h"
#include "textureatlas.h"

namespace dukat
{
	class TextureCache
	{
	private:
		const std::string resource_dir;
		// Flip surfaces vertically / horizontally by default. False by default.
		// Set vflip to true to change images from natural orientation (y-axis points down)
		// to OpenGL standard (y-axis points up). 
		bool hflip;
		bool vflip;
		uint32_t last_id;
		std::unordered_map<uint32_t, std::unique_ptr<Texture>> textures;
		std::unordered_map<std::string, std::unique_ptr<Surface>> surfaces;
		std::unordered_map<std::string, std::unique_ptr<TextureAtlas>> atlases;
		std::unique_ptr<Texture> load(const std::string& filename, TextureFilterProfile profile);

	public:
		// Creates a new texture cache.
		TextureCache(const std::string& resource_dir);
		~TextureCache(void);

		// Helper method to load a surface from a file.
		Surface* load_surface(const std::string& filename, bool hflip = false, bool vflip = false);
		void free_surface(const std::string& filename) { if (surfaces.count(filename)) surfaces.erase(filename); }
		// Generates an ID that can be used to store and retrieve dynamic textures and surfaces.
		uint32_t generate_id(void) { return ++last_id; }

		// Returns a texture for a image file.
		Texture* get(const std::string& filename, TextureFilterProfile profile = ProfileNearest);
		Texture* get(uint32_t id);
		
		// Returns a texture with a specific id if it exists in the cache.
		Texture* find_by_texture_id(const TextureId id) const;

		// Puts texture entry into cache.
		Texture* put(const std::string& filename, std::unique_ptr<Texture> texture);
		Texture* put(uint32_t id, std::unique_ptr<Texture> texture);
		// Frees all cached textures.
		void free_all(void);
		// Frees the texture corresponding to the image file.
		void free(const std::string& filename) { free(compute_hash(filename)); }
		void free(uint32_t id);

		// Looks up texture index by name.
		TextureAtlas* get_atlas(const std::string& name);

		void set_hflip(bool hflip) { this->hflip = hflip; }
		void set_vflip(bool vflip) { this->vflip = vflip; }
	};
}
