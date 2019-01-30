#include "stdafx.h"
#include "fractalscene.h"

namespace dukat
{
	constexpr int FractalScene::texture_size;

	FractalScene::FractalScene(Game3* game) : game(game)
	{
		MeshBuilder2 mb2;
		if (!game->get_meshes()->contains("quad"))
		{
			game->get_meshes()->put("quad", mb2.build_textured_quad());
		}

		// Generate framebuffer and texture
		fbo = std::make_unique<FrameBuffer>(texture_size, texture_size, true, false);
		fb_program = game->get_shaders()->get_program("fx_default.vsh", "fx_fractal.fsh");

		overlay_meshes.stage = RenderStage::OVERLAY;
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
		ss << "<WASD> Move" << std::endl
			<< "<LMB> Zoom in" << std::endl
			<< "<RMB> Zoom out" << std::endl
			<< "<F11> Toggle Info" << std::endl
			<< "<ESC> Quit" << std::endl
			<< std::endl;
		info_text->set_text(ss.str());
		info_text->transform.update();
		info_mesh = overlay_meshes.add_instance(std::move(info_text));
	}

	void FractalScene::activate(void)
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

	void FractalScene::handle_keyboard(const SDL_Event & e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->pop_scene();
			break;
		case SDLK_F11:
			info_mesh->visible = !info_mesh->visible;
			break;
		}
	}

	void FractalScene::update(float delta)
	{
		overlay_meshes.update(delta);
		update_framebuffer(delta);
	}

	void FractalScene::update_framebuffer(float delta)
	{
		static float increment = 2.0f;
		static float zoom = 1.0f;
		static Vector2 pos{ 0.0f, 0.0f };
		
		game->get_renderer()->set_blending(false);

		fbo->bind();

		auto ctrl = game->get_devices()->active;
		if (ctrl->is_pressed(InputDevice::Button1))
		{
			zoom += increment * delta;
			increment *= 1.01f;
		}
		else if (zoom > 0.1f && ctrl->is_pressed(InputDevice::Button2))
		{
			zoom -= increment * delta;
			increment *= 0.99f;
		}
		pos.x += -ctrl->lx / zoom * delta;
		pos.y += -ctrl->ly / zoom * delta;

        game->get_renderer()->switch_shader(fb_program);
		fb_program->set("u_k", 0.35f, 0.4f);
		fb_program->set("u_offset", pos.x, pos.y);
		fb_program->set("u_zoom", zoom);
        game->get_meshes()->get("quad")->render(fb_program);

		fbo->unbind();

		game->get_renderer()->set_blending(true);
		perfc.inc(PerformanceCounter::FRAME_BUFFERS);
	}

	void FractalScene::render(void)
	{
		std::vector<Mesh*> meshes;
		meshes.push_back(&overlay_meshes);
		meshes.push_back(game->get_debug_meshes());
		game->get_renderer()->render(meshes);
	}
}
