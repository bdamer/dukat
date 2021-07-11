#pragma once

namespace dukat
{
	class AABB2;
	class RenderLayer2;
	class Renderer2;

	class Effect2
	{
	protected:
		RenderLayer2* layer;
		std::function<void(Effect2*)> update_handler;

	public:
		Effect2(void) : layer(nullptr), update_handler(nullptr) { }
		virtual ~Effect2(void) { }

		// Called to render this effect on the current layer.
		virtual void render(Renderer2* renderer, const AABB2& camera_bb) = 0;
		void set_layer(RenderLayer2* layer) { this->layer = layer; }
		RenderLayer2* get_layer(void) const { return layer; }
		void set_update_handler(const std::function<void(Effect2*)> handler) { this->update_handler = handler; }
	};
}