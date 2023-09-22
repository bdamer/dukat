#pragma once

namespace dukat
{
	class GameBase;

	// Generic manager interface.
	class Manager
	{
	protected:
		GameBase* game;
		bool enabled;

	public:
		Manager(GameBase* game) : game(game), enabled(true) { }
		virtual ~Manager(void) { }

		// Called once per frame to update internal state.
		virtual void update(float delta) { };

		virtual void set_enabled(bool enabled) { this->enabled = enabled; }
		bool is_enabled(void) const { return enabled; }
	};
}