#include "stdafx.h"
#include <dukat/causticseffect2.h>
#include <dukat/renderlayer2.h>
#include <dukat/renderer2.h>
#include <dukat/game2.h>
#include <dukat/meshbuilder2.h>

namespace dukat
{
	CausticsEffect2::CausticsEffect2(Game2* game, ShaderProgram* sp, int width, int height, float velocity)
		: game(game), sp(sp), velocity(velocity), color_set(0)
	{
		fbo = std::make_unique<FrameBuffer>(width, height, true, false, TextureFilterProfile::ProfileNearest);
		MeshBuilder2 mb2;
		fb_quad = mb2.build_textured_quad();
	}

	void CausticsEffect2::resize(int width, int height)
	{
		fbo->resize(width, height);
	}

	void CausticsEffect2::render(Renderer2* renderer, const AABB2& camera_bb)
	{
		fbo->bind();
		renderer->switch_shader(sp);

		sp->set("u_time", velocity * game->get_time() + 23.0f);

		switch (color_set)
		{
		case 0: // Blue
			sp->set("u_color_lo", 0.051f, 0.09f, 0.286f, 1.f);	// #0d1749
			sp->set("u_color_mid", 0.047f, 0.08f, 0.576f, 1.f);	// #0c0293
			sp->set("u_color_hi", 0.188f, 0.012f, 0.851f, 1.f);	// #3003d9
			break;
		case 1: // Red
			sp->set("u_color_lo", .769f, 0.141f, 0.188f, 1.f);	// #ffeb57
			sp->set("u_color_mid", 1.f, 0.314f, 0.0f, 1.f);		// #ff5000
			sp->set("u_color_hi", 1.f, 0.922f, 0.341f, 1.f);	// #c42430
			break;
		case 2: // Green
			sp->set("u_color_lo", 0.047f, 0.180f, 0.267f, 1.f);	// #0c2e44
			sp->set("u_color_mid", 0.075f, 0.298f, 0.298f, 1.f);// #134c4c
			sp->set("u_color_hi", 0.353f, 0.773f, 0.310f, 1.f);	// #5ac54f
			break;
		}

		if (update_handler != nullptr)
			update_handler(this);

		renderer->clear();
		fb_quad->render(sp);
		fbo->unbind();
		perfc.inc(PerformanceCounter::FRAME_BUFFERS);
	}

	Texture* CausticsEffect2::get_texture(void) const
	{
		return fbo->texture.get();
	}
}