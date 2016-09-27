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

	protected:
		bool done;
		std::unique_ptr<Window> window;
		std::unique_ptr<DeviceManager> device_manager;
		int last_fps;
		const Settings& settings;

	public:
		Application(const Settings& settings);
		virtual ~Application(void);
		// Runs the main application loop
		int run(void);
		// Called to initialize the application.
		virtual void init(void) = 0;
		// Called to process input events.
		virtual void handle_event(const SDL_Event& e) = 0;
		// Called to update the application state.
		virtual void update(float delta) = 0;
		// Called to render to the screen.
		virtual void render(void) = 0;

		Window* get_window(void) const { return window.get(); }
		DeviceManager* get_devices(void) const { return device_manager.get(); }
	};
}