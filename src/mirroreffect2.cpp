#include "stdafx.h"
#include <dukat/mirroreffect2.h>
#include <dukat/renderlayer2.h>
#include <dukat/renderer2.h>
#include <dukat/game2.h>

namespace dukat
{
	void MirrorEffect2::render(Renderer2* renderer, const AABB2& camera_bb)
	{
		auto target_layer = renderer->get_layer(mirrored_layer);
		if (target_layer == nullptr)
			return;

		const AABB2 shifted_bb{ camera_bb.min + shift, camera_bb.max + shift };

		// swap sprite program and render sprites
		auto sp = target_layer->get_sprite_program();
		target_layer->set_sprite_program(sprite_program);
		target_layer->render_sprites(renderer, shifted_bb, [](Sprite* s) { return (s->flags & Sprite::fx) != Sprite::fx; });
		target_layer->set_sprite_program(sp);

		// do the same for particles
		auto pp = layer->get_particle_program();
		target_layer->set_particle_program(particle_program);
		target_layer->render_particles(renderer, shifted_bb);
		target_layer->set_particle_program(pp);
	}
}