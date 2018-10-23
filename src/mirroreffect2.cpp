#include "stdafx.h"
#include <dukat/mirroreffect2.h>
#include <dukat/renderlayer2.h>
#include <dukat/renderer2.h>
#include <dukat/game2.h>

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

		// do the same for particles
		auto pp = layer->get_particle_program();
		layer->set_particle_program(particle_program);
		layer->render_particles(renderer, camera_bb);
		layer->set_particle_program(pp);
	}
}