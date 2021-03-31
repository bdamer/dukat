// surfaceapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "surfaceapp.h"

namespace dukat
{
	SurfaceScene::SurfaceScene(Game2* game2) : Scene2(game2)
	{
		auto settings = game->get_settings();
		auto layer = game->get_renderer()->create_composite_layer("main", 1.0f);

		// Set up default camera centered around origin
		auto camera = std::make_unique<Camera2>(game, Vector2(texture_width, texture_height));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		// Initialize texture once
		texture = std::make_unique<Texture>(texture_width, texture_height);
		Rect r = { 0, 0, texture_width, texture_height };
		sprite = std::make_unique<Sprite>(texture.get(), r);
		layer->add(sprite.get());

		// Set up info text
		auto info_layer = game->get_renderer()->create_direct_layer("overlay", 25.0f);
		info_text = game->create_text_mesh();
		info_text->set_size(8.0f);
		info_text->transform.position = Vector3(-0.5f * (float)texture_width, 0.0f, 0.0f);
		info_text->transform.update();
		std::stringstream ss;
		ss << "Surface Test" << std::endl
			<< "Press 1: Load test image" << std::endl
			<< "Press 2: B/W test image" << std::endl
			<< "Press 3: Color test image" << std::endl
			<< std::endl
			<< "Press B: Blend image" << std::endl
			<< "Press H: Flip image horizontally" << std::endl
			<< "Press V: Flip image horizontally" << std::endl;
		info_text->set_text(ss.str());
		info_layer->add(info_text.get());

		// Set up debug layer
		auto debug_layer = game->get_renderer()->create_direct_layer("debug", 1000.0f);
		debug_text = game->create_text_mesh();
		debug_text->set_size(4.0f);
		debug_text->transform.position = Vector3(-0.5f * (float)texture_width, -0.5f * (float)texture_height, 0.0f);
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

		game->set_controller(this);
	}

	void SurfaceScene::handle_keyboard(const SDL_Event& e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;
		case SDLK_1:
			test_load_image();
			break;
		case SDLK_2:
			test_bw_image();
			break;
		case SDLK_3:
			test_color_image();
			break;
		case SDLK_b:
			if (surface != nullptr)
			{
				auto img = Surface::from_file("../assets/textures/test_rgb24.png");
				surface->blend(*img);
				texture->load_data(*surface, TextureFilterProfile::ProfileNearest);
			}
			break;
		case SDLK_h:
			if (surface != nullptr)
			{
				surface->flip_horizontal();
				texture->load_data(*surface, TextureFilterProfile::ProfileNearest);
			}
			break;
		case SDLK_v:
			if (surface != nullptr)
			{
				surface->flip_vertical();
				texture->load_data(*surface, TextureFilterProfile::ProfileNearest);
			}
			break;
		}
	}
	
	void SurfaceScene::test_load_image(void)
	{
		surface = Surface::from_file("../assets/textures/test_rgb24.png");
		texture->load_data(*surface, TextureFilterProfile::ProfileNearest);
	}

	void SurfaceScene::test_bw_image(void)
	{
		surface = std::make_unique<Surface>(texture_width, texture_height, SDL_PIXELFORMAT_RGBA8888);
		for (int x = 0; x < texture_width; x++)
		{
			for (int y = 0; y < texture_height; y++)
			{
				auto dist = (x - texture_width / 2) * (x - texture_width / 2)
					+ (y - texture_height / 2) * (y - texture_height / 2);
				auto grad = 2 * 100;
				auto alpha = (Uint8)(255.0f * (1.0f - (float)dist / (float)grad));
				surface->set_pixel(x, y, surface->color(0xff, 0xff, 0xff, alpha));
			}
		}
		texture->load_data(*surface, TextureFilterProfile::ProfileNearest);
	}

	void SurfaceScene::test_color_image(void)
	{
		surface = std::make_unique<Surface>(texture_width, texture_height, SDL_PIXELFORMAT_RGBA8888);
		surface->fill_rect(0, 0, 64, 64, surface->color(0, 0, 0, 255));
		surface->fill_rect(64, 0, 64, 64, surface->color(255, 0, 0, 255));
		surface->fill_rect(128, 0, 64, 64, surface->color(0, 255, 0, 255));
		surface->fill_rect(192, 0, 64, 64, surface->color(0, 0, 255, 255));
		surface->fill_rect(0, 64, 64, 64, surface->color(255, 255, 0, 255));
		surface->fill_rect(64, 64, 64, 64, surface->color(255, 0, 255, 255));
		surface->fill_rect(128, 64, 64, 64, surface->color(0, 255, 255, 255));
		surface->fill_rect(192, 64, 64, 64, surface->color(255, 255, 255, 255));
		surface->fill_rect(0, 128, 64, 64, surface->color(0, 0, 0, 127));
		surface->fill_rect(64, 128, 64, 64, surface->color(255, 0, 0, 127));
		surface->fill_rect(128, 128, 64, 64, surface->color(0, 255, 0, 127));
		surface->fill_rect(192, 128, 64, 64, surface->color(0, 0, 255, 127));
		surface->fill_rect(0, 192, 64, 64, surface->color(255, 255, 0, 127));
		surface->fill_rect(64, 192, 64, 64, surface->color(255, 0, 255, 127));
		surface->fill_rect(128, 192, 64, 64, surface->color(0, 255, 255, 127));
		surface->fill_rect(192, 192, 64, 64, surface->color(255, 255, 255, 127));
		texture->load_data(*surface, TextureFilterProfile::ProfileNearest);
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/surface.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game2 app(settings);
		app.add_scene("main", std::make_unique<dukat::SurfaceScene>(&app));
		app.push_scene("main");
		return app.run();
	}
	catch (const std::exception& e)
	{
		dukat::log->error("Application failed with error: {}", e.what());
		return -1;
	}
}