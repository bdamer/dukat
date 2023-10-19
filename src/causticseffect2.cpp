#include "stdafx.h"
#include <dukat/causticseffect2.h>
#include <dukat/game2.h>
#include <dukat/meshbuilder2.h>
#include <dukat/renderlayer2.h>
#include <dukat/renderer2.h>
#include <dukat/textureutil.h>

namespace dukat
{
	CausticsEffect2::CausticsEffect2(Game2* game, ShaderProgram* sp, int width, int height, float velocity)
		: game(game), sp(sp), velocity(velocity), color_set(0)
	{
		TextureBuilder tb;
		texture = tb.set_filter_profile(ProfileNearest)
			.set_wrap(GL_CLAMP_TO_BORDER)
			.set_internal_format(GL_R8)
			.set_format(GL_RED)
			.set_type(GL_FLOAT)
			.set_width(width)
			.set_height(height)
			.build();

		fbo = std::make_unique<FrameBuffer>(width, height, false, false, TextureFilterProfile::ProfileNearest);

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
		fbo->attach_draw_buffer(texture.get());
		renderer->switch_shader(sp);

		sp->set("u_time", velocity * game->get_time() + 23.0f);

		if (update_handler != nullptr)
			update_handler(this);

		renderer->clear();
		fb_quad->render(sp);
		fbo->unbind();
		perfc.inc(PerformanceCounter::FRAME_BUFFERS);
	}

	Texture* CausticsEffect2::get_texture(void) const
	{
		return texture.get();
	}
}