// framebufferapp.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "framebufferapp.h"

namespace dukat
{
	constexpr int Game::texture_size;

	void Game::init(void)
	{
		Game3::init();

		// Top-down camera
		auto camera = std::make_unique<FixedCamera3>(this, Vector3{ 0.0f, 2.5f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3::unit_z);
		camera->set_vertical_fov(settings.get_float("camera.fov"));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		renderer->set_camera(std::move(camera));

		MeshBuilder2 mb2;
		mesh_cache->put("quad", mb2.build_textured_quad());

		// Generate framebuffer and texture
		fbo = std::make_unique<FrameBuffer>(texture_size, texture_size, true, false);

		fb_program = shader_cache->get_program("fx_animation.vsh", "fx_animation.fsh");

		overlay_meshes.stage = RenderStage::OVERLAY;
		overlay_meshes.visible = true;

		quad_mesh = overlay_meshes.create_instance();
		quad_mesh->transform.position.z = 0.5f;
		quad_mesh->set_mesh(mesh_cache->get("quad"));
		quad_mesh->set_texture(fbo->texture.get());
		quad_mesh->set_program(shader_cache->get_program("sc_ui_texture.vsh", "sc_texture.fsh"));

		auto info_text = create_text_mesh(1.0f / 20.0f);
		info_text->transform.position = { -1.5f, -0.5f, 0.5f };
		std::stringstream ss;
		ss << "<#magenta><F1> Nothing" << std::endl
			<< "<F11> Toggle Info" << std::endl
			<< std::endl;
		info_text->set_text(ss.str());
		info_text->transform.update();
		info_mesh = overlay_meshes.add_instance(std::move(info_text));

		debug_meshes.stage = RenderStage::OVERLAY;
		debug_meshes.visible = debug;

		auto debug_text = create_text_mesh(1.0f / 20.0f);
		debug_text->transform.position = { -1.5f, 0.8f, 0.0f };
		debug_text->transform.update();
		debug_meshes.add_instance(std::move(debug_text));
	}

	void Game::handle_keyboard(const SDL_Event & e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_F11:
			info_mesh->visible = !info_mesh->visible;
			break;
		
		default:
			Game3::handle_keyboard(e);
		}
	}

	void Game::update(float delta)
	{
		Game3::update(delta);
		overlay_meshes.update(delta);
		debug_meshes.update(delta);
		update_framebuffer(delta);
	}

	void Game::update_framebuffer(float delta)
	{
        glDisable(GL_BLEND);

        fbo->bind();

        renderer->switch_shader(fb_program);
		fb_program->set("u_time", get_time());
        mesh_cache->get("quad")->render(fb_program);

        fbo->unbind();

        glEnable(GL_BLEND);
        perfc.inc(PerformanceCounter::FRAME_BUFFERS);
	}

	void Game::render(void)
	{
		std::vector<Mesh*> meshes;
		meshes.push_back(&debug_meshes);
		meshes.push_back(&overlay_meshes);
		renderer->render(meshes);
	}

	void Game::toggle_debug(void)
	{
		debug_meshes.visible = !debug_meshes.visible;
	}

	void Game::update_debug_text(void)
	{
		std::stringstream ss;
		auto cam = renderer->get_camera();
		ss << "WIN: " << window->get_width() << "x" << window->get_height()
			<< " FPS: " << get_fps()
			<< " MESH: " << dukat::perfc.avg(dukat::PerformanceCounter::MESHES)
			<< " VERT: " << dukat::perfc.avg(dukat::PerformanceCounter::VERTICES) << std::endl;
		auto debug_text = dynamic_cast<TextMeshInstance*>(debug_meshes.get_instance(0));
		debug_text->set_text(ss.str());
	}

	void Game::release(void)
	{
		Game3::release();
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/framebuffer.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game app(settings);
		return app.run();
	}
	catch (const std::exception& e)
	{
		dukat::logger << "Application failed with error." << std::endl << e.what() << std::endl;
		return -1;
	}
	return 0;
}