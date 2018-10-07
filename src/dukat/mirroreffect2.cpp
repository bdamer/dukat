#include "stdafx.h"
#include "mirroreffect2.h"
#include "renderlayer2.h"
#include "renderer2.h"
#include "game2.h"

namespace dukat
{
	void MirrorEffect2::render(Renderer2* renderer, const AABB2& camera_bb)
	{
		auto layer = renderer->get_layer(mirrored_layer);
		if (layer == nullptr)
			return;

		// swap sprite program and render sprites
		auto sp = layer->get_sprite_program();
		layer->set_sprite_program(sprite_program);
		layer->render_sprites(renderer, camera_bb);
		layer->set_sprite_program(sp);
	}
}