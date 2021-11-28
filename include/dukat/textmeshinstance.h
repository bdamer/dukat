#pragma once

#include <string>
#include "bitmapfont.h"
#include "meshinstance.h"

namespace dukat
{
	class TextMeshInstance : public MeshInstance
	{
	public:
		enum Align
		{
			Center,
			Left,
			Right,
			Top, 
			Bottom
		};

	private:
		static constexpr auto vertices_per_char = 6;
		static const auto max_length = 1024;

		const float yorientation;
		// Unlike a regular mesh instance, a text mesh instance does not
		// use the mesh cache. Instead it stores the mesh data directly.
		std::unique_ptr<MeshData> text_mesh;
		std::string text;
		// scales space allocated for characters and lines
		float char_width;
		float line_height;
		// If > 0, will add line breaks to preserve max width.
		// 1 unit = 1 character at default width
		float max_line_width;
		// unscaled width and height
		float width, height;
		// number of vertices in current mesh
		int num_vertices;
		BitmapFont* font;
		// delay between characters
		float scroll_delay;
		float scroll_accumulator;
		std::function<void(void)> scroll_callback;
		bool rebuild_required;

		void create_mesh(void);
		void update_scroll(float delta);
		void rebuild(void);

	public:
		Align halign; // Valid values: Center, Left, Right
		Align valign; // Valid values: Center, Top, Bottom

		TextMeshInstance(BitmapFont* font, float yorientation = 1.0f);
		~TextMeshInstance(void) { }

		// Accessors
		void set_text(const std::string& text);
		std::string get_text(void) const { return text; }
		void set_alpha(float alpha);
		float get_alpha(void) const;
		void set_color(const Color& color) { set_ambient(color); }
		Color get_color(void) const { return get_ambient(); }
		void set_size(float size) { transform.scale = { size, size, size }; }
		float get_size(void) const { return transform.scale.x; }
		void set_char_width(float w) { char_width = w; rebuild_required = true; }
		float get_char_width(void) const { return char_width; }
		void set_line_height(float h) { line_height = h; rebuild_required = true; }
		float get_line_height(void) const { return line_height; }
		void set_max_line_width(float w) { max_line_width = w; rebuild_required = true; }
		float get_max_line_width(void) const { return max_line_width; }
		void set_font(BitmapFont* font);
		BitmapFont* get_font(void) const { return font; }
		float get_width(void) const { return width * transform.scale.x; }
		float get_height(void) const { return height * transform.scale.x; }

		// Initiates scroll for the provided text.
		void set_text_scroll(const std::string& text, float delay, const std::function<void(void)>& callback = nullptr);
		void cancel_scroll(void) { scroll_delay = 0.0f; scroll_callback = nullptr; }

		void update(float delta = 0.0f);
	};
}