#include "stdafx.h"
#include <dukat/application.h>
#include <dukat/audiomanager.h>
#include <dukat/log.h>
#include <dukat/perfcounter.h>
#include <dukat/sdlutil.h>
#include <dukat/sysutil.h>
#include <dukat/window.h>
#include <dukat/devicemanager.h>
#include <dukat/inputrecorder.h>
#include <dukat/keyboarddevice.h>
#include <dukat/rand.h>
#include <dukat/settings.h>
#include <ctime>

// Define to enable slow frame tracing
// #define PERF_TRACE

namespace dukat
{
	constexpr float Application::max_frame_delta;

	Application::Application(Settings& settings)
		: title(settings.get_string("window.title")), runtime(0.0f), fixed_frame_rate(0.0f),
		paused(false), active(true), done(false), last_update(0), settings(settings)
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
		window = std::make_unique<Window>(settings);
		window->set_title(title);

		auto show_cursor = settings.get_bool("input.mouse.cursor", false);
		if (!show_cursor)
			SDL_ShowCursor(SDL_DISABLE);

#ifndef __ANDROID__
		audio_manager = std::make_unique<AudioManager>(settings.get_int("audio.channels", 16));
		audio_manager->set_music_volume(settings.get_float("audio.music.volume", 1.0f));
		audio_manager->set_sample_volume(settings.get_float("audio.sample.volume", 1.0f));
#endif

		// Initialize random generator once
		rand::seed(time(nullptr));

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
#ifdef PERF_TRACE
		uint64_t start_ticks, update_ticks, render_ticks;
#endif
		float delta;
		SDL_Event e;
		while (!done)
		{
			ticks = SDL_GetTicks();
#ifdef PERF_TRACE
			start_ticks = SDL_GetPerformanceCounter();
#endif
			if (paused || !active)
			{
				delta = 1.0f / 15.0f;
				SDL_Delay(static_cast<Uint32>(1000 / 15));
			}
			else if (fixed_frame_rate > 0.0f)
			{
				delta = fixed_frame_rate;
				runtime += fixed_frame_rate;
				update(std::min(fixed_frame_rate, max_frame_delta));
			}
			else
			{
				delta = static_cast<float>(ticks - last_update) / 1000.0f;
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
			
			device_manager->update(delta);

#ifdef PERF_TRACE
			update_ticks = SDL_GetPerformanceCounter();
#endif

			// render to screen
			render();

#ifdef PERF_TRACE
			render_ticks = SDL_GetPerformanceCounter();
			const auto freq = static_cast<double>(SDL_GetPerformanceFrequency());
			const auto update_time = static_cast<double>(update_ticks - start_ticks) / freq;
			const auto render_time = static_cast<double>(render_ticks - update_ticks) / freq;
			const auto total_time = update_time + render_time;
			if (!paused && active && total_time > 0.02)
				log->warn("Slow frame: {} [{} {}]", total_time, update_time, render_time);
#endif
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
				window->toggle_fullscreen();
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
			active = true;
			if (settings.get_bool("input.mouse.lock", true))
				sdl_check_result(SDL_SetRelativeMouseMode(SDL_TRUE), "Set mouse mode");
			break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
			active = false;
			if (settings.get_bool("input.mouse.lock", true))
				sdl_check_result(SDL_SetRelativeMouseMode(SDL_FALSE), "Set mouse mode");
	        break;
		case SDL_WINDOWEVENT_SHOWN:
			active = true;
			break;
		case SDL_WINDOWEVENT_HIDDEN:
			active = false;
			break;
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			window->on_resize();
			if (paused) // run a single frame update to fix display if paused
				update(1.0f / 60.0f);
			break;
		}
	}
}