#pragma once

#include <memory>
#include <string>
#include "messenger.h"

namespace dukat
{
	class Window;
	class AudioManager;
	class DeviceManager;
	class Settings;

	class Application : public Messenger
	{
	private:
		// max duration between two frames. if the actual duration is greater, 
		// the update function will be called with this value.
		static constexpr auto max_frame_delta = 1.0f / 15.0f;
		const std::string title; // Window title
		int last_fps; // Most recent FPS value
		float fixed_frame_rate; // Fix frame rate to use instead of actual delta
		float runtime; // time since program start in seconds
		bool paused; // Triggered by application events. If set, will not execute update method.
		bool active; // Triggered by application window becoming active. If set, will execute update method.
		bool done; // If set, will exit application

	protected:
		uint32_t last_update; // Ticks at last update
		Settings& settings;
		std::unique_ptr<Window> window;
#ifndef __ANDROID__
		std::unique_ptr<AudioManager> audio_manager;
#endif
		std::unique_ptr<DeviceManager> device_manager;

		// Called to process input events.
		virtual void handle_event(const SDL_Event& e);
		// Called to handle keyboard events.
		virtual void handle_keyboard(const SDL_Event& e);
		// Called to handle window events.
		virtual void handle_window_event(const SDL_Event& e);
		// Called to update the application state.
		virtual void update(float delta) = 0;
		// Called to render to the screen.
		virtual void render(void) = 0;
		// Takes a screenshot of current frame.
		virtual void save_screenshot(const std::string& filename) = 0;

	public:
		// Called to initialize the application.
		Application(Settings& settings);
		// Called to destroy the application.
		virtual ~Application(void);

		// Runs the main application loop
		int run(void);

		void toggle_pause(void) { paused = !paused; }
		bool is_paused(void) const { return paused; }
		void set_paused(bool paused) { this->paused = paused; }
		bool is_done(void) const { return done; }
		void set_done(bool done) { this->done = done; }
		void set_fixed_frame_rate(float rate) { fixed_frame_rate = rate; }
		int get_fps(void) const { return last_fps; }
		float get_time(void) const { return runtime; }

		Window* get_window(void) const { return window.get(); }
#ifndef __ANDROID__
		AudioManager* get_audio(void) const { return audio_manager.get(); }
#endif
		DeviceManager* get_devices(void) const { return device_manager.get(); }
		Settings& get_settings(void) const { return settings; }
	};
}