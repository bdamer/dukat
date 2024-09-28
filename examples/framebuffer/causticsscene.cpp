#include "stdafx.h"
#include "causticsscene.h"

namespace dukat
{
	constexpr int CausticsScene::texture_size;

	CausticsScene::CausticsScene(Game3* game) : game(game), speed(1.f), time(0.0f), color_set(0)
	{
		MeshBuilder2 mb2;
		if (!game->get_meshes()->contains("quad"))
			game->get_meshes()->put("quad", mb2.build_textured_quad());

		// Generate framebuffer and texture
		fbo = std::make_unique<FrameBuffer>(texture_size, texture_size, true, false);
		fb_program = game->get_shaders()->get_program("fx_default.vsh", "fx_caustics.fsh");

		overlay_meshes.stage = RenderStage::Overlay;
		overlay_meshes.visible = true;

		quad_mesh = overlay_meshes.create_instance();
		quad_mesh->transform.position.z = 0.5f;
		quad_mesh->set_mesh(game->get_meshes()->get("quad"));
		quad_mesh->set_texture(fbo->texture.get());
		quad_mesh->set_program(game->get_shaders()->get_program("sc_ui_texture.vsh", "sc_texture.fsh"));

		auto info_text = game->create_text_mesh();
		info_text->set_size(1.0f / 20.0f);
		info_text->transform.position = { -1.5f, -0.5f, 0.5f };
		std::stringstream ss;
		ss << "<C> Cycle color" << std::endl
			<< "<,.> Change Speed" << std::endl
			<< "<F11> Toggle Info" << std::endl
			<< "<ESC> Quit" << std::endl
			<< std::endl;
		info_text->set_text(ss.str());
		info_text->update();
		info_mesh = overlay_meshes.add_instance(std::move(info_text));
	}

	void CausticsScene::activate(void)
	{
		auto settings = game->get_settings();
		// Top-down camera
		auto camera = std::make_unique<FixedCamera3>(game, Vector3{ 0.0f, 2.5f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3::unit_z);
		camera->set_vertical_fov(settings.get_float(settings::video_camera_fov));
		camera->set_clip(settings.get_float(settings::video_camera_nearclip), settings.get_float(settings::video_camera_farclip));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		game->set_controller(this);
	}

	void CausticsScene::handle_keyboard(const SDL_Event & e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->pop_scene();
			break;
		case SDLK_c:
			color_set = (color_set + 1) % 3;
			break;
		case SDLK_COMMA:
			speed = std::max(0.5f * speed, 1.0f);
			break;
		case SDLK_PERIOD:
			speed = std::min(2.f * speed, 128.f);
			break;
		case SDLK_F11:
			info_mesh->visible = !info_mesh->visible;
			break;
		}
	}

	void CausticsScene::update(float delta)
	{
		overlay_meshes.update(delta);
		update_framebuffer(delta);
	}

	void CausticsScene::update_framebuffer(float delta)
	{
		game->get_renderer()->set_blending(false);

		fbo->bind();

		time += speed * delta;

        game->get_renderer()->switch_shader(fb_program);
		fb_program->set("u_time", time);

		switch (color_set)
		{
		case 0: // Blue
			fb_program->set("u_color_lo", 0.051f, 0.09f, 0.286f, 1.f);	// #0d1749
			fb_program->set("u_color_mid", 0.047f, 0.08f, 0.576f, 1.f);	// #0c0293
			fb_program->set("u_color_hi", 0.188f, 0.012f, 0.851f, 1.f);	// #3003d9
			break;
		case 1: // Red
			fb_program->set("u_color_lo", .769f, 0.141f, 0.188f, 1.f);	// #ffeb57
			fb_program->set("u_color_mid", 1.f, 0.314f, 0.0f, 1.f);		// #ff5000
			fb_program->set("u_color_hi", 1.f, 0.922f, 0.341f, 1.f);	// #c42430
			break;
		case 2: // Green
			fb_program->set("u_color_lo", 0.047f, 0.180f, 0.267f, 1.f);	// #0c2e44
			fb_program->set("u_color_mid", 0.075f, 0.298f, 0.298f, 1.f);// #134c4c
			fb_program->set("u_color_hi", 0.353f, 0.773f, 0.310f, 1.f);	// #5ac54f
			break;
		}

	    game->get_meshes()->get("quad")->render(fb_program);

		fbo->unbind();

		game->get_renderer()->reset_viewport();
		game->get_renderer()->set_blending(true);
		perfc.inc(PerformanceCounter::FRAME_BUFFERS);
	}

	void CausticsScene::render(void)
	{
		std::vector<Mesh*> meshes;
		meshes.push_back(&overlay_meshes);
		meshes.push_back(game->get_debug_meshes());
		game->get_renderer()->render(meshes);
	}
}
