#include "stdafx.h"
#include <dukat/shadoweffect2.h>
#include <dukat/renderlayer2.h>
#include <dukat/renderer2.h>
#include <dukat/game2.h>

namespace dukat
{
	void ShadowEffect2::render(Renderer2* renderer, const AABB2& camera_bb)
	{
		// swap sprite program and render sprites
		auto sp = layer->get_sprite_program();
		layer->set_sprite_program(sprite_program);
		// force activation so parameter can be set
		renderer->switch_shader(sprite_program);
		sprite_program->set("u_alpha", alpha);
		sprite_program->set("u_radius", radius);
		layer->render_sprites(renderer, camera_bb, [](Sprite* s) { return (s->flags & Sprite::fx) != Sprite::fx; });
		layer->set_sprite_program(sp);
	}
}