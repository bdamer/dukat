#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class Player;

	class Game : public Game3
	{
	private:
		MeshGroup debug_meshes;
		MeshGroup object_meshes;
		
		void init(void);
		void update(float delta);
		void update_debug_text(void);
		void render(void);

	public:
		Game(Settings& settings) : Game3(settings) { }
		~Game(void) { }

		void toggle_debug(void);
	};
}