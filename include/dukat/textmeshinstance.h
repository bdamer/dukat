#pragma once

#include <string>
#include "meshinstance.h"

namespace dukat
{
	class TextMeshInstance : public MeshInstance
	{
	public:
		enum Align
		{
			Left, Center, Right
		};

	private:
		// Unlike a regular mesh instance, a text mesh instance does not
		// use the mesh cache. Instead it stores the mesh data directly.
		std::unique_ptr<MeshData> text_mesh;
		std::string text;
		// unscaled width and height
		float width, height;

	public:
		Align align;

		TextMeshInstance(std::unique_ptr<MeshData> text_mesh);
		~TextMeshInstance(void) { }

		void set_text(const std::string& text);
		std::string get_text(void) const { return text; }
		void set_alpha(float alpha);
		float get_alpha(void) const;
		void set_size(float size);
		float get_width(void) const { return width * transform.scale.x; }
		float get_height(void) const { return height * transform.scale.x; }
		void update(float delta);
	};
}