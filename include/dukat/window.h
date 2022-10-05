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

		enum Flags
		{
			Fullscreen = 1,
			FullscreenDesktop = 2,
			Borderless = 4,
			Resizable = 8
		};

		// Physical resolution
		int width;
		int height;
		int flags;
		bool vsync;
		bool msaa_enabled;

		SDL_Window* window;
		SDL_GLContext context;

		void set_context_attributes(bool msaa);
		void create_window(void);
		void create_context(void);

	public:
		enum class Mode
		{
			Windowed,
			Borderless,
			Fullscreen
		};

		Window(const Settings& settings);
		~Window(void);

		// Called by application to update screen buffer.
		void present(void);
		
		void set_title(const std::string& title) { SDL_SetWindowTitle(window, title.c_str()); }
		void set_vsync(bool vsync);
		void set_display_mode(const SDL_DisplayMode& mode);
		void set_fullscreen(bool fullscreen);
		void toggle_fullscreen(void) { set_fullscreen(!(flags & Fullscreen)); }
		void set_borderless(bool borderless);

		void change_mode(Mode wm, const SDL_DisplayMode& dm);
		void resize(int width, int height);
		int get_height(void) const { return height; }
		int get_width(void) const { return width; }
		float get_aspect_ratio(void) const { return (float)width / (float)height; }

		bool is_fullscreen(void) const { return flags & Fullscreen; }
		bool is_borderless(void) const { return flags & Borderless; }
		bool is_resizable(void) const { return flags & Resizable; }
		bool is_msaa_enabled(void) const { return msaa_enabled; }
		
		// Triggered by application / SDL after resize.
		void on_resize(void);
	};
}