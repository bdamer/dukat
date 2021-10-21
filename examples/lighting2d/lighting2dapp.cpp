// lighting2dapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "lighting2dapp.h"

namespace dukat
{
	LightingApp::LightingApp(Game2* game2) : Scene2(game2)
	{
		auto settings = game->get_settings();

		auto window = game->get_window();
		auto renderer = game->get_renderer();

		// Adjust scene render stage to use custom composition
		auto scene_stage = renderer->get_stage(RenderStage::Scene);
		scene_stage->composite_program = game->get_shaders()->get_program("fx_default.vsh", "fx_lighting.fsh");
		scene_stage->composite_binder = [&](ShaderProgram* p) {
			p->set("u_ambient", 0.05f, 0.05f, 0.05f, 1.0f);
		};
		scene_stage->frame_buffer = std::make_unique<FrameBuffer>(window->get_width(), window->get_height(),
			true, false, TextureFilterProfile::ProfileNearest);

		// Set up default camera centered around origin
		auto camera = std::make_unique<Camera2>(game);
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->set_resize_handler(fixed_height_camera(camera_height));
		camera->refresh();
		renderer->set_camera(std::move(camera));

		const auto ratio = static_cast<float>(window->get_height()) / static_cast<float>(window->get_width());
		// adjust width to nearest multiple of 2
		auto camera_width = static_cast<int>(std::round(static_cast<float>(camera_height) / ratio));
		camera_width += camera_width % 2;

		// Set up info text
		auto info_layer = renderer->create_direct_layer("overlay", 25.0f);
		info_text = game->create_text_mesh();
		info_text->set_size(2.0f);
		info_text->transform.position = Vector3(-0.45f * (float)camera_width, 0.4f * (float)camera_height, 0.0f);
		info_text->transform.update();
		std::stringstream ss;
		ss << "Lighting Example" << std::endl
			<< "G - Toggle gamma correction" << std::endl;
		info_text->set_text(ss.str());
		info_layer->add(info_text.get());

		// Set up debug layer
		auto debug_layer = renderer->create_direct_layer("debug", 1000.0f);
		debug_text = game->create_text_mesh();
		debug_text->set_size(4.0f);
		debug_text->transform.position = Vector3(-0.5f * (float)camera_width, -0.5f * (float)camera_height, 0.0f);
		debug_text->transform.update();
		debug_layer->add(debug_text.get());
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

		create_layers();
		create_lights();

		game->set_controller(this);
	}

	void LightingApp::create_layers(void)
	{
		auto renderer = game->get_renderer();

		// Ground tiles
		ground_texture = game->get_textures()->get("ground32.png");

		// Set up layers
		auto ground_layer = game->get_renderer()->create_composite_layer("ground", 0.0f);
		ground_layer->set_composite_program(game->get_shaders()->get_program("fx_default.vsh", "fx_ground.fsh"));
		ground_layer->set_composite_binder([&](ShaderProgram* p) {
			ground_texture->bind(1, p);
			p->set("u_scale", 8.0f);
		});

		auto lights_layer = renderer->create_composite_layer("lights", 10.0f);
		auto light_tex = game->get_textures()->get("light.png");
		light_sprites[0] = std::make_unique<Sprite>(light_tex, Rect{ 0, 0, 8, 8 });
		lights_layer->add(light_sprites[0].get());
		light_sprites[1] = std::make_unique<Sprite>(light_tex, Rect{ 8, 0, 8, 8 });
		lights_layer->add(light_sprites[1].get());
		light_sprites[2] = std::make_unique<Sprite>(light_tex, Rect{ 0, 8, 8, 8 });
		lights_layer->add(light_sprites[2].get());
		light_sprites[3] = std::make_unique<Sprite>(light_tex, Rect{ 8, 8, 8, 8 });
		lights_layer->add(light_sprites[3].get());
	}

	void LightingApp::create_lights(void)
	{
		auto renderer = game->get_renderer();

		// blue
		auto light0 = renderer->get_light(0);
		light0->color = Color{ 0.0471f, 0.9451f, 1.0f, 1.0f };
		set_light_range(*light0, 100.0f);

		// green
		auto light1 = renderer->get_light(1);	
		light1->color = Color{ 0.6f, 0.902f, 0.3725f, 1.0f };
		set_light_range(*light1, 200.0f);

		// magenta
		auto light2 = renderer->get_light(2);
		light2->color = Color{ 0.9529f, 0.5373f, 0.9608f, 1.0f };
		set_light_range(*light2, 300.0f);
		
		// red
		auto light3 = renderer->get_light(3);
		light3->color = Color{ 0.9176f, 0.1961f, 0.2553f, 1.0f };
		set_light_range(*light3, 400.0f);
	}

	void LightingApp::update(float delta)
	{
		static auto accu = 0.0f;
		accu += delta;

		// Fixed lights
		light_sprites[0]->p = Vector2{ 0, 0 };
		light_sprites[1]->p = Vector2{ -50, 50 };

		// Orbit light
		light_sprites[2]->p = Vector2{ 0, 1 }.rotate(accu) * 50.0f;

		// Horizontal movement
		light_sprites[3]->p.x = std::sin(accu) * 100.0f;

		auto renderer = game->get_renderer();
		for (auto i = 0; i < num_lights; i++)
			renderer->get_light(i)->position = light_sprites[i]->p;
	}

	void LightingApp::handle_keyboard(const SDL_Event& e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;
		case SDLK_g:
			game->get_renderer()->set_gamma_correct(!game->get_renderer()->is_gamma_correct());
			break;
		}
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/lighting2d.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game2 app(settings);
		app.add_scene("main", std::make_unique<dukat::LightingApp>(&app));
		app.push_scene("main");
		return app.run();
	}
	catch (const std::exception& e)
	{
		dukat::log->error("Application failed with error: {}", e.what());
		return -1;
	}
}