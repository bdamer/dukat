#pragma once

#include <vector>
#include <SDL2/SDL.h>

namespace dukat
{
	class WindowEventListener
	{
	public:
		virtual void resize(int width, int height) = 0;
	};

	class Window
	{
	private:
		// Physical resolution
		int width;
		int height;
		bool fullscreen;
		bool msaa_enabled;
		SDL_Window* window;
		SDL_GLContext context;
		std::vector<WindowEventListener*> event_listeners;

		void on_resize(void);

	public:
		Window(int width = 640, int height = 480, bool fullscreen = false, bool msaa = false);
		~Window();

		void clear(void);
		void resize(int width, int height);
		void set_fullscreen(bool fullscreen);
		void toggle_fullscreen(void);
		void set_title(const std::string& title);
		void set_vsync(bool vsync);
		int get_height(void) const { return height; }
		int get_width(void) const { return width; }
		float get_aspect_ratio(void) const { return (float)width / (float)height; }
		bool is_msaa_enabled(void) const { return msaa_enabled; }

		void present(void);

		void bind(WindowEventListener* listener) { event_listeners.push_back(listener); }
		void unbind(WindowEventListener* listener);		
	};
}