#pragma once

#include <memory>
#include <string>

namespace dukat
{
	class Window;
	class DeviceManager;
	class Settings;

	class Application
	{
	private:
		const std::string title;
		int last_fps;
		bool paused;
		bool done;

	protected:
		const Settings& settings;
		std::unique_ptr<Window> window;
		std::unique_ptr<DeviceManager> device_manager;

		// Called to initialize the application.
		virtual void init(void) = 0;
		// Called to process input events.
		virtual void handle_event(const SDL_Event& e);
		// Called to handle keyboard events.
		virtual void handle_keyboard(const SDL_Event& e);
		// Called to update the application state.
		virtual void update(float delta) = 0;
		// Called to render to the screen.
		virtual void render(void) = 0;
		// Called to release the application.
		virtual void release(void) = 0;

	public:
		Application(const Settings& settings);
		virtual ~Application(void);

		// Runs the main application loop
		int run(void);

		void toggle_pause(void) { paused = !paused; }
		bool is_done(void) const { return done; }
		void set_done(bool done) { this->done = done; }
		int get_fps(void) const { return last_fps; }

		Window* get_window(void) const { return window.get(); }
		DeviceManager* get_devices(void) const { return device_manager.get(); }
	};
}