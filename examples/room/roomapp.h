#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class Player;

	class RoomScene : public Scene, public Controller
	{
	private:
		Game3* game;
		MeshGroup object_meshes;

	public:
		RoomScene(Game3* game);
		~RoomScene(void) { }

		void update(float delta);
		void render(void);
	};
}