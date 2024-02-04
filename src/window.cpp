#include "stdafx.h"
#include <dukat/log.h>
#include <dukat/sdlutil.h>
#include <dukat/string.h>
#include <dukat/sysutil.h>
#include <dukat/window.h>

namespace dukat
{
	Window::Window(const Settings& settings) : flags(0)
	{
		this->width = settings.get_int("window.width", -1);
		this->height = settings.get_int("window.height", -1);

		if (settings.get_bool("window.fullscreen"))
			flags |= Flags::Fullscreen;
		if (settings.get_bool("window.fullscreen_desktop"))
			flags |= Flags::FullscreenDesktop;
		if (settings.get_bool("window.resizable"))
			flags |= Flags::Resizable;
		if (settings.get_bool("window.borderless"))
			flags |= Flags::Borderless;

		const auto msaa = settings.get_bool("window.msaa");
		set_context_attributes(msaa);
		create_window();
		create_context();

		// Set vsync for current context 
		set_vsync(settings.get_bool("window.vsync", true));

#ifdef OPENGL_CORE
		if (msaa)
		{
			// Check that MSAA buffers are available
			int msaa_buffers, msaa_samples;
			SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &msaa_buffers);
			SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &msaa_samples);
			msaa_enabled = msaa_buffers > 0 && msaa_samples > 0;
			glEnable(GL_MULTISAMPLE);
		}
#endif

#ifndef __ANDROID__
		// init glew
		glewExperimental = GL_TRUE;
		auto res = glewInit();
		assert(res == GLEW_OK);
		// ignore GL_INVALID_ENUM after glewInit(), see
		// http://www.opengl.org/wiki/OpenGL_Loading_Library	
		auto error = glGetError();
		assert(error == GL_NO_ERROR || error == GL_INVALID_ENUM);
#endif

#ifdef _DEBUG
		gl_check_error();
#endif
	}

	Window::~Window()
	{
		if (window != nullptr && (flags & Flags::Fullscreen)) // leave fullscreen to restore original res
			toggle_fullscreen();

		log->debug("Destroying window.");
		if (context != nullptr)
		{
			SDL_GL_DeleteContext(context);
			context = nullptr;
		}
		if (window != nullptr)
		{
			SDL_DestroyWindow(window);
			window = nullptr;
		}
	}

	void Window::set_context_attributes(bool msaa)
	{
#ifdef OPENGL_CORE
		// Need to request MSAA buffers before creating window
		if (msaa)
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 1);
		}

		// Request sRGB capable frame buffer to support gamma correction on Intel cards
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
#endif

		// Create OpenGL context with desired profile version
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_MAJOR_VERSION);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_MINOR_VERSION);
#ifdef OPENGL_CORE
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#elif OPENGL_ES
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
	}

	void Window::create_window(void)
	{
#ifdef __ANDROID__
		// Ignore provided width & height - reuse existing dimensions
		width = height = -1;
#endif

		const auto display_names = list_display_names();
		log->debug("Available displays: {}", join(display_names));

		SDL_DisplayMode cur_display_mode;
		SDL_GetCurrentDisplayMode(0, &cur_display_mode);
		if (width == -1 || height == -1)
		{
			width = cur_display_mode.w;
			height = cur_display_mode.h;
		}

		log->debug("Creating window with size: {}x{}", width, height);
		Uint32 window_flags;
#ifdef __ANDROID__
		// Ignore provided width & height - reuse existing dimensions
		SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeRight");
		window_flags = SDL_WINDOW_SHOWN;
#else
		window_flags = SDL_WINDOW_OPENGL;
		if (flags & Flags::Fullscreen)
		{
			if (flags & Flags::FullscreenDesktop)
				window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
			else
				window_flags |= SDL_WINDOW_FULLSCREEN;
		}
		else
		{
			window_flags |= SDL_WINDOW_OPENGL;
			if (flags & Flags::Resizable)
				window_flags |= SDL_WINDOW_RESIZABLE;
			if (flags & Flags::Borderless)
				window_flags |= SDL_WINDOW_BORDERLESS;
		}
#endif
		// Create the window with the requested resolution
		window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			width, height, window_flags);
		if (window == nullptr)
			sdl_check_result(-1, "Create SDL Window");

		if ((flags & Flags::Fullscreen) && !(flags & Flags::FullscreenDesktop) && (width != cur_display_mode.w || height != cur_display_mode.h))
		{
			SDL_DisplayMode target, closest;
			target.w = width;
			target.h = height;
			target.format = 0;
			target.refresh_rate = 0;
			target.driverdata = 0; 

			log->debug("Finding closed display mode to: {}x{}", width, height);

			if (SDL_GetClosestDisplayMode(0, &target, &closest) == nullptr)
			{
				log->warn("Unable to find matching display mode: {}x{}", width, height);
			}
			else
			{
				log->info("Found mode: {}", format_display_mode(closest));
				change_mode(Mode::Fullscreen, closest);
			}
		}
	}

	void Window::create_context(void)
	{
		context = SDL_GL_CreateContext(window);
		if (context == nullptr)
			sdl_check_result(-1, "Create OpenGL Context");
		int major, minor;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
		log->debug("Created OpenGL context {}.{}", major, minor);
	}

	void Window::change_mode(Mode wm, const SDL_DisplayMode& dm)
	{
		const auto cur_mode = is_fullscreen() ? Mode::Fullscreen :
			(is_borderless() ? Mode::Borderless : Mode::Windowed);
		if (cur_mode != wm)
		{
			switch (wm)
			{
			case Mode::Windowed:
				set_fullscreen(false);
				set_borderless(false);
				break;
			case Mode::Borderless:
				set_fullscreen(false);
				set_borderless(true);
				break;
			case Mode::Fullscreen:
				set_fullscreen(true);
				break;
			}
		}

		if (is_fullscreen())
			set_display_mode(dm);
		else
			resize(dm.w, dm.h);
	}

	void Window::resize(int width, int height)
	{
		// Note: resize only affects the window when not using fullscreen
		log->debug("Resizing window to: {}x{}", width, height);
		this->width = width;
		this->height = height;
		SDL_SetWindowSize(window, width, height);
		SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}

	void Window::set_display_mode(const SDL_DisplayMode& mode)
	{
		// Note: display mode changes the underlying display, but not the window
		log->debug("Switching to display mode: {}", format_display_mode(mode));
		this->width = mode.w;
		this->height = mode.h;
		sdl_check_result(SDL_SetWindowDisplayMode(window, &mode), "Change Display Mode");
		SDL_SetWindowSize(window, width, height); // always change window size to avoid visual artifacts
	}

	void Window::set_fullscreen(bool fullscreen)
	{
		uint32_t target_flags;
		if (fullscreen)
		{
			flags |= Fullscreen;
			target_flags = flags & FullscreenDesktop ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
		}
		else
		{
			flags &= ~Fullscreen;
			target_flags = 0;
		}
		log->debug("Switching to {}", (fullscreen ? "fullscreen" : "window"));
		sdl_check_result(SDL_SetWindowFullscreen(window, target_flags), "Change screen mode");

		if (!fullscreen) // re-center position upon leaving fullscreen mode
			SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

		// On Linux, seeing duplicate SDL events get sent after window resize. 
		// Introduce slight delay to avoid this...
		SDL_Delay(100);
	}

	void Window::set_borderless(bool borderless)
	{
		if (borderless)
			flags |= Borderless;
		else
			flags &= ~Borderless;
		SDL_SetWindowBordered(window, !borderless ? SDL_TRUE : SDL_FALSE);
	}

	void Window::set_vsync(bool vsync)
	{
		this->vsync = vsync;
	#if SDL_VERSION_ATLEAST(1,3,0)
			SDL_GL_SetSwapInterval(vsync);
	#else /* SDL_VERSION_ATLEAST(1,3,0) */
		#ifdef SDL_GL_SWAP_CONTROL
				SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, vsync);
		#else /* SDL_GL_SWAP_CONTROL */
				DEBUG("VSync unsupported on old SDL versions (before 1.2.10).");
		#endif /* SDL_GL_SWAP_CONTROL */
	#endif /* SDL_VERSION_ATLEAST(1,3,0) */
	}

	void Window::on_resize(void)
	{
		SDL_GetWindowSize(window, &width, &height);
		log->debug("Window resized to: {}x{}", width, height);
		trigger(Message{events::WindowResized, &width, &height});
	}

	void Window::present(void)
	{
		SDL_GL_SwapWindow(window); 
	}
}