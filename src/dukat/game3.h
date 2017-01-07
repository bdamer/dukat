#pragma once

#include "gamebase.h"

namespace dukat
{
	class MeshCache;
	class Renderer3;
	class TextMeshInstance;

	class Game3 : public GameBase
	{
	protected:
		std::unique_ptr<Renderer3> renderer;

		virtual void update(float delta);

	public:
		Game3(const Settings& settings);
		~Game3(void) { }

		Renderer3* get_renderer(void) const { return renderer.get(); }
	};
}