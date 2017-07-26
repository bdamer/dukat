#pragma once

namespace dukat
{
	class AABB2;
	class RenderLayer2;
	class Renderer2;

	class Effect2
	{
	private:
		RenderLayer2* layer;

	public:
		Effect2(void) { }
		virtual ~Effect2(void) { }

		// Called to render this effect on the current layer.
		virtual void render(Renderer2* renderer, const AABB2& camera_bb) = 0;
		void set_layer(RenderLayer2* layer) { this->layer = layer; }
		RenderLayer2* get_layer(void) const { return layer; }
	};
}