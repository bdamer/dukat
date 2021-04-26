#include "stdafx.h"
#include <dukat/application.h>
#include <dukat/audiomanager.h>
#include <dukat/log.h>
#include <dukat/perfcounter.h>
#include <dukat/sdlutil.h>
#include <dukat/sysutil.h>
#include <dukat/window.h>
#include <dukat/devicemanager.h>
#include <dukat/keyboarddevice.h>
#include <dukat/settings.h>
#include <ctime>

namespace dukat
{
	constexpr float Application::max_frame_delta;

	Application::Application(Settings& settings)
		: title(settings.get_string("window.title")), runtime(0.0f), paused(false), done(false), last_update(0), settings(settings)
	{
		init_logging(settings);
		log->info("Initializing application.");

		SDL_version compiled;
		SDL_version linked;
		SDL_VERSION(&compiled);
		SDL_GetVersion(&linked);
		log->debug("SDL version {}.{}.{} ({}.{}.{})", 
			static_cast<int>(compiled.major), static_cast<int>(compiled.minor), static_cast<int>(compiled.patch),
			static_cast<int>(linked.major), static_cast<int>(linked.minor), static_cast<int>(linked.patch));

		sdl_check_result(SDL_Init(SDL_INIT_EVERYTHING), "Initialize SDL");
		window = std::make_unique<Window>(settings.get_int("window.width", 640), settings.get_int("window.height", 480),
			settings.get_bool("window.fullscreen"), settings.get_bool("window.vsync", true), settings.get_bool("window.msaa"));
		window->set_title(title);

		auto show_cursor = settings.get_bool("input.mouse.cursor", false);
		if (!show_cursor)
			SDL_ShowCursor(SDL_DISABLE);

#ifndef __ANDROID__
		audio_manager = std::make_unique<AudioManager>(settings.get_int("audio.channels", 16));
		audio_manager->set_music_volume(settings.get_float("audio.music.volume", 1.0f));
		audio_manager->set_sample_volume(settings.get_float("audio.sample.volume", 1.0f));
#endif

		device_manager = std::make_unique<DeviceManager>(*window, settings);
		device_manager->add_keyboard();
		gl_check_error();
	}

	Application::~Application(void)
	{
		// Force release of device manager before call to SDL_Quit
		device_manager = nullptr;
		// Always show cursor before exiting
		SDL_ShowCursor(SDL_ENABLE);
		SDL_Quit();
	}

	int Application::run(void)
	{
		log->info("Entering application loop.");
		uint32_t ticks, last_frame = 0u;
		SDL_Event e;
		while (!done)
		{
			ticks = SDL_GetTicks();

			if (paused)
			{
				SDL_Delay(static_cast<Uint32>(1000 / 15));
			}
			else 
			{
				auto delta = static_cast<float>(ticks - last_update) / 1000.0f;
				runtime += delta;
				update(std::min(delta, max_frame_delta));
			}
			last_update = ticks;

			// update FPS counter
			if (ticks - last_frame >= 1000)
			{
				last_fps = perfc.sum(PerformanceCounter::FRAMES);
				last_frame = ticks;
				perfc.collect_stats();
			}

			// process events
			while (SDL_PollEvent(&e))
			{
				handle_event(e);
			}

			device_manager->update();

			// render to screen
			render();
			perfc.inc(PerformanceCounter::FRAMES);
			perfc.reset();
		}

		return 0;
	}

	void Application::handle_event(const SDL_Event& e)
	{
		switch (e.type)
		{
		case SDL_QUIT:
			done = true;
			break;
		case SDL_KEYDOWN:
			handle_keyboard(e);
			break;
		case SDL_WINDOWEVENT:
			handle_window_event(e);
			break;
		case SDL_JOYDEVICEADDED:
			device_manager->add_joystick(e.jdevice.which);
			break;
		case SDL_JOYDEVICEREMOVED:
			device_manager->remove_joystick(e.jdevice.which);
			break;
		}
	}

	void Application::handle_keyboard(const SDL_Event& e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_RETURN:
			if (e.key.keysym.mod & KMOD_ALT)
			{
				window->toggle_fullscreen();
			}
			break;
		case SDLK_PAUSE:
			toggle_pause();
			break;
		case SDLK_PRINTSCREEN:
			{
				std::stringstream ss;
				ss << "screenshot_" << std::time(nullptr) << ".png";
				save_screenshot(ss.str());
			}
			break;
		}
	}

	void Application::handle_window_event(const SDL_Event& e)
	{
		switch (e.window.event)
		{
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			paused = false;
			if (settings.get_bool("input.mouse.lock", true))
			{
				sdl_check_result(SDL_SetRelativeMouseMode(SDL_TRUE), "Set mouse mode");
			}
			break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
			paused = true;
			if (settings.get_bool("input.mouse.lock", true))
			{
				sdl_check_result(SDL_SetRelativeMouseMode(SDL_FALSE), "Set mouse mode");
			}
	        break;
		case SDL_WINDOWEVENT_SHOWN:
			paused = false;
			break;
		case SDL_WINDOWEVENT_HIDDEN:
			paused = true;
			break;
		case SDL_WINDOWEVENT_RESIZED:
			window->on_resize();
			break;
		}
	}
}