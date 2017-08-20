#pragma once

#include <memory>
#include <string>

namespace dukat
{
	class MeshData;
	struct Color;

	// Utility class to generate meshes to render text based on a sprite map.
	class TextMeshBuilder
	{
	private:
		// Max length in characters
		static const int max_length = 1024;
		// Number of rows / columns in the character map.
		const int cols;
		const int rows;
		Uint32 simple_hash(const std::string& name) const;

		// attempts to parse color from tag
		bool parse_color(size_t& i, const std::string& text, Color& color) const;

	public:
		// Creates a new text mesh builder for a sprite map with cols * rows characters.
		TextMeshBuilder(int cols = 16, int rows = 16);
		~TextMeshBuilder(void) { }
		std::unique_ptr<MeshData> build_text_mesh(const std::string& text = "") const;
		void rebuild_text_mesh(MeshData* mesh, const std::string& text, float& width, float& height) const;
	};
}