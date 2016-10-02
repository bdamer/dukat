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

		std::unique_ptr<TextMeshInstance> create_text_mesh(float size = 1.0f, const Color& color = { 1.0f, 1.0f, 1.0f, 1.0f });
		Renderer3* get_renderer(void) const { return renderer.get(); }
	};
}