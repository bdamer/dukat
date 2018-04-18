#pragma once

namespace dukat
{
	class GameBase;

	// Generic manager interface.
	class Manager
	{
	protected:
		GameBase* game;

	public:
		Manager(GameBase* game) : game(game) { }
		virtual ~Manager(void) { }

		// Called once per frame to update internal state.
		virtual void update(float delta) { };
	};
}