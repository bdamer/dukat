#pragma once

namespace dukat
{
	class Controller
	{
	public:
		Controller(void) { }
		virtual ~Controller(void) { }
		// Handles application event.
		virtual void handle_event(const SDL_Event& e) { }
		// Handles keyboard event.
		virtual void handle_keyboard(const SDL_Event& e) { }
	};
}