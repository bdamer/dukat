#include "stdafx.h"
#include "ripplepondscene.h"

namespace dukat
{
	constexpr int RipplePondScene::texture_size;
	constexpr float RipplePondScene::ripple_amplitude;
	constexpr float RipplePondScene::ripple_dampening;

	RipplePondScene::RipplePondScene(Game3* game) : game(game)
	{
		MeshBuilder2 mb2;
		if (!game->get_meshes()->contains("quad"))
		{
			game->get_meshes()->put("quad", mb2.build_textured_quad());
		}

		background_idx = 0;
		background[0] = game->get_textures()->get_or_load("box01_1024.png");
		background[1] = game->get_textures()->get_or_load("rock01_1024.png");
		background[2] = game->get_textures()->get_or_load("sand01_1024.png");
		background[3] = game->get_textures()->get_or_load("grass01_1024.png");
		background[4] = game->get_textures()->get_or_load("dirt01_1024.png");

		// Generate framebuffer
		fbo = std::make_unique<FrameBuffer>(texture_size, texture_size, false, false);
		fb_program = game->get_shaders()->get_program("fx_default.vsh", "fx_ripple.fsh");

		// Generate 2 buffers for ripple map
		TextureBuilder tb;
		tex0 = tb.set_filter_profile(ProfileLinear)
			.set_wrap(GL_CLAMP_TO_BORDER)
			.set_internal_format(GL_R32F)
			.set_format(GL_RED)
			.set_type(GL_FLOAT)
			.set_width(texture_size)
			.set_height(texture_size)
			.build();
		current = tex0.get();

		tex1 = tb.build();
		last = tex1.get();

		overlay_meshes.stage = RenderStage::Overlay;
		overlay_meshes.visible = true;

		// Create quad mesh & set material properties
		quad_mesh = overlay_meshes.create_instance();
		quad_mesh->transform.position.z = 0.5f;
		quad_mesh->set_mesh(game->get_meshes()->get("quad"));
		quad_mesh->set_program(game->get_shaders()->get_program("sc_ui_texture.vsh", "sc_ripple.fsh"));
		auto mat = quad_mesh->get_material();
		mat.custom.r = ripple_amplitude;
		mat.custom.g = static_cast<float>(texture_size);
		mat.custom.b = 1.0f / static_cast<float>(texture_size);
		mat.custom.a = refraction_index;
		quad_mesh->set_material(mat);

		auto info_text = game->create_text_mesh();
		info_text->set_size(1.0f / 20.0f);
		info_text->transform.position = { -1.5f, -0.5f, 0.5f };
		std::stringstream ss;
		ss << "<LMB> Add ripple" << std::endl
			<< "<D> Add random drop" << std::endl
			<< "<T> Cycle texture" << std::endl
			<< "<F11> Toggle Info" << std::endl
			<< "<ESC> Quit" << std::endl
			<< std::endl;
		info_text->set_text(ss.str());
		info_text->update();
		info_mesh = overlay_meshes.add_instance(std::move(info_text));
	}

	void RipplePondScene::activate(void)
	{
		auto settings = game->get_settings();
		// Top-down camera
		auto camera = std::make_unique<FixedCamera3>(game, Vector3{ 0.0f, 2.5f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3::unit_z);
		camera->set_vertical_fov(settings.get_float("camera.fov"));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		game->set_controller(this);
	}

	void RipplePondScene::add_ripple(int x, int y, float amplitude)
	{
		last->bind(0);
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, GL_RED, GL_FLOAT, &amplitude);
		last->unbind();
	}

	void RipplePondScene::handle_keyboard(const SDL_Event & e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->pop_scene();
			break;
		case SDLK_d:
			add_ripple(std::rand() % texture_size, std::rand() % texture_size, random(0.25f * ripple_amplitude, ripple_amplitude));
			break;
		case SDLK_t:
			background_idx++;
			if (background_idx > 4)
				background_idx = 0;
			break;
		case SDLK_F11:
			info_mesh->visible = !info_mesh->visible;
			break;
		}
	}

	void RipplePondScene::update(float delta)
	{
		auto ctrl = game->get_devices()->active;
		if (ctrl->is_pressed(InputDevice::Button1))
		{
			// this assumes that ripple map fills the whole height of the window
			const auto window_width = static_cast<float>(game->get_window()->get_width());
			const auto window_height = static_cast<float>(game->get_window()->get_height());

			// convert x coordinate and restrict to visible range
			auto x = (ctrl->rxa - (0.5f * window_width - 0.5f * window_height)) / window_height; // normalized x
			x = std::max(std::min(x, 1.0f), 0.0f);
			auto y = 1.0f - (ctrl->rya / window_height); // normalized y

			add_ripple(static_cast<int>(x * texture_size), static_cast<int>(y * texture_size), random(-ripple_amplitude, 0.0f));
		}

		overlay_meshes.update(delta);
		update_framebuffer(delta);
	}

	void RipplePondScene::update_framebuffer(float delta)
	{		
		game->get_renderer()->set_blending(false);

		fbo->bind();
		fbo->attach_draw_buffer(current);
		game->get_renderer()->switch_shader(fb_program);
		current->bind(0, fb_program);
		last->bind(1, fb_program);
		const auto size = static_cast<float>(texture_size);
		if (fb_program->attr("u_size") > -1) fb_program->set("u_size", size);
		fb_program->set("u_one_over_size", 1.0f / size);
		fb_program->set("u_dampening", ripple_dampening);
		game->get_meshes()->get("quad")->render(fb_program);

		fbo->unbind();

		game->get_renderer()->reset_viewport();
		game->get_renderer()->set_blending(true);
		perfc.inc(PerformanceCounter::FRAME_BUFFERS);
	}

	void RipplePondScene::render(void)
	{
		quad_mesh->set_texture(background[background_idx], 0);
		quad_mesh->set_texture(current, 1);

		std::vector<Mesh*> meshes;
		meshes.push_back(&overlay_meshes);
		meshes.push_back(game->get_debug_meshes());
		game->get_renderer()->render(meshes);

		// Swap ripple buffers
		std::swap(current, last);
	}
}
