#pragma once

#include <vector>

#ifndef OPENGL_VERSION
#include "version.h"
#endif // !OPENGL_VERSION

#include "messenger.h"
#include "settings.h"

namespace dukat
{
	class Window : public Messenger
	{
	private:
		static constexpr auto window_title = "dukat";

		// Physical resolution
		int width;
		int height;
		bool fullscreen;
		bool resizable; // indicates that the window can be resized
		bool vsync;
		bool msaa_enabled;
		SDL_Window* window;
		SDL_GLContext context;

		void set_context_attributes(void);
		void create_window(void);

	public:
		Window(const Settings& settings);
		~Window(void);

		// Called by application to update screen buffer.
		void present(void);
		
		void set_title(const std::string& title) { SDL_SetWindowTitle(window, title.c_str()); }
		void set_vsync(bool vsync);
		void set_fullscreen(bool fullscreen);
		void toggle_fullscreen(void) { set_fullscreen(!fullscreen); }

		void resize(int width, int height);
		int get_height(void) const { return height; }
		int get_width(void) const { return width; }
		float get_aspect_ratio(void) const { return (float)width / (float)height; }

		bool is_fullscreen(void) const { return fullscreen; }
		bool is_resizable(void) const { return resizable; }
		bool is_msaa_enabled(void) const { return msaa_enabled; }
		
		// Triggered by application / SDL after resize.
		void on_resize(void);
	};
}