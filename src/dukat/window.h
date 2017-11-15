#pragma once

#include <vector>
#include <SDL2/SDL.h>

#ifndef OPENGL_VERSION
#include "version.h"
#endif // !OPENGL_VERSION

#include "messenger.h"

namespace dukat
{
	class Window : public Messenger
	{
	private:
		// Physical resolution
		int width;
		int height;
		bool fullscreen;
		bool vsync;
		bool msaa_enabled;
		SDL_Window* window;
		SDL_GLContext context;

	public:
		Window(int width, int height, bool fullscreen, bool vsync, bool msaa);
		~Window();

		// Clears screen buffers.
		void clear(void) { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }
		// Called by application to update screen buffer.
		void present(void) { SDL_GL_SwapWindow(window); }
		
		void set_title(const std::string& title) { SDL_SetWindowTitle(window, title.c_str()); }
		void set_vsync(bool vsync);
		void set_fullscreen(bool fullscreen);
		void toggle_fullscreen(void) { set_fullscreen(!fullscreen); }

		void resize(int width, int height);
		int get_height(void) const { return height; }
		int get_width(void) const { return width; }
		float get_aspect_ratio(void) const { return (float)width / (float)height; }

		bool is_fullscreen(void) const { return fullscreen; }
		bool is_msaa_enabled(void) const { return msaa_enabled; }
		
		// Triggered by application / SDL after resize.
		void on_resize(void);
	};
}