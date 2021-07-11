#include "stdafx.h"
#include <dukat/shadoweffect2.h>
#include <dukat/renderlayer2.h>
#include <dukat/renderer2.h>
#include <dukat/game2.h>

namespace dukat
{
	void ShadowEffect2::render(Renderer2* renderer, const AABB2& camera_bb)
	{
		auto target_layer = renderer->get_layer(shadowed_layer);
		if (target_layer == nullptr)
			return;

		if (update_handler != nullptr)
			update_handler(this);

		// swap sprite program and render sprites
		auto sp = target_layer->get_sprite_program();
		target_layer->set_sprite_program(sprite_program);
		// force activation so parameter can be set
		renderer->switch_shader(sprite_program);
		sprite_program->set("u_alpha", alpha);
		sprite_program->set("u_radius", radius);
		target_layer->render_sprites(renderer, camera_bb, [](Sprite* s) { return (s->flags & Sprite::fx) != Sprite::fx; });
		target_layer->set_sprite_program(sp);
	}
}