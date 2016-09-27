#pragma once

namespace dukat
{
	struct Texture;

	// Loads a texture from a DDS file.
	std::unique_ptr<Texture> load_dds(const std::string& filename);
	// Loads a texture from a DDS stream.
	std::unique_ptr<Texture> load_dds(std::istream& is);
}