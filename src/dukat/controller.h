#pragma once

namespace dukat
{
	class Controller
	{
	public:
		Controller(void) { }
		virtual ~Controller(void) { }
		// Handles event and returns true if event was processed.
		virtual bool handle_event(const SDL_Event& e) { return false; }
		// Handles keyboard event and returns true if event was processed.
		virtual bool handle_keyboard(const SDL_Event& e) { return false; }
	};
}