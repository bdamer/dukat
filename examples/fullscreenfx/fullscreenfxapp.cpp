// fullscreenfxapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "fullscreenfxapp.h"

namespace dukat
{
	FullscreenFXScene::FullscreenFXScene(Game2* game2) : Scene2(game2)
	{
		auto settings = game->get_settings();

		// Set up default camera centered around origin
		auto camera = std::make_unique<Camera2>(game);
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->set_resize_handler(fixed_width_camera(texture_width));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		init_debug();
		init();

		game->set_controller(this);
	}

	void FullscreenFXScene::init(void)
	{
		auto layer = game->get_renderer()->create_composite_layer("main", 1.0f);

		// Load sample image
		auto texture = game->get_textures()->get("sunset.jpg");
		sprite = std::make_unique<Sprite>(texture);
		layer->add(sprite.get());

		// Set up info text
		auto info_layer = game->get_renderer()->create_direct_layer("overlay", 25.0f);
		info_text = game->create_text_mesh();
		info_text->set_size(16.0f);
		info_text->transform.position = Vector3(-0.5f * (float)texture_width, 0.0f, 0.0f);
		info_text->transform.update();
		std::stringstream ss;
		ss << "FullscreenFX" << std::endl
			<< "Press 1: Invert colors" << std::endl
			<< "Press 2: Wave Distortion" << std::endl
			<< "Press 3: Color shift" << std::endl
			<< "Press 4: Color reduction" << std::endl
			<< "Press 0: Reset" << std::endl
			<< std::endl;
		info_text->set_text(ss.str());
		info_layer->add(info_text.get());
	}

	void FullscreenFXScene::init_debug(void)
	{
		// Set up debug layer
		const auto texture_height = static_cast<int>(texture_width / game->get_window()->get_aspect_ratio());
		auto debug_layer = game->get_renderer()->create_direct_layer("debug", 1000.0f);
		debug_text = game->create_text_mesh();
		debug_text->set_size(8.0f);
		debug_text->transform.position = Vector3(-0.5f * (float)texture_width, -0.5f * (float)texture_height, 0.0f);
		debug_text->transform.update();
		debug_layer->add(debug_text.get());
		if (!game->is_debug())
			debug_layer->hide();

		game->get<TimerManager>()->create_timer(1.0f, [&]() {
			std::stringstream ss;
			auto window = game->get_window();
			auto cam = game->get_renderer()->get_camera();
			ss << "WIN: " << window->get_width() << "x" << window->get_height()
				<< " VIR: " << cam->transform.dimension.x << "x" << cam->transform.dimension.y
				<< " FPS: " << game->get_fps()
				<< " MESH: " << dukat::perfc.avg(dukat::PerformanceCounter::MESHES)
				<< " VERT: " << dukat::perfc.avg(dukat::PerformanceCounter::VERTICES) << std::endl;
			debug_text->set_text(ss.str());
			}, true);
	}

	void FullscreenFXScene::handle_keyboard(const SDL_Event& e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;
		case SDLK_r:
			game->get_renderer()->destroy_layers();
			init_debug();
			init();
			break;

		case SDLK_0:
			game->get_renderer()->set_composite_program(game->get_shaders()->get_program("fx_default.vsh", "fx_default.fsh"));
			break;
		case SDLK_1:
			game->get_renderer()->set_composite_program(game->get_shaders()->get_program("fx_default.vsh", "fx_invert.fsh"));
			break;
		case SDLK_2:
			game->get_renderer()->set_composite_program(game->get_shaders()->get_program("fx_default.vsh", "fx_wave.fsh"), [&](ShaderProgram* p) {
				p->set("u_phase", game->get_time());
			});
			break;
		case SDLK_3:
			game->get_renderer()->set_composite_program(game->get_shaders()->get_program("fx_default.vsh", "fx_clrshift.fsh"), [&](ShaderProgram* p) {
				p->set("u_phase", game->get_time());
			});
			break;
		case SDLK_4:
			game->get_renderer()->set_composite_program(game->get_shaders()->get_program("fx_default.vsh", "fx_clredux.fsh"));
			break;
		}
	}	
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/fullscreenfx.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game2 app(settings);
		app.add_scene("main", std::make_unique<dukat::FullscreenFXScene>(&app));
		app.push_scene("main");
		return app.run();
	}
	catch (const std::exception& e)
	{
		dukat::log->error("Application failed with error: {}", e.what());
		return -1;
	}
}