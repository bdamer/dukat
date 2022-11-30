// surfaceapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "surfaceapp.h"

namespace dukat
{
	#pragma region Palettes

	static const std::vector<Color> bw_palette = {
		Color{ 0.0f, 0.0f, 0.0f, 1.0f },
		Color{ 1.0f, 1.0f, 1.0f, 1.0f },
	};

	static const std::vector<Color> color_palette = {
		color_rgb(255, 0, 64),
		color_rgb(19, 19, 19),
		color_rgb(27, 27, 27),
		color_rgb(39, 39, 39),
		color_rgb(61, 61, 61),
		color_rgb(93, 93, 93),
		color_rgb(133, 133, 133),
		color_rgb(180, 180, 180),
		color_rgb(255, 255, 255),
		color_rgb(199, 207, 221),
		color_rgb(146, 161, 185),
		color_rgb(101, 115, 146),
		color_rgb(66, 76, 110),
		color_rgb(42, 47, 78),
		color_rgb(26, 25, 50),
		color_rgb(14, 7, 27),
		color_rgb(28, 18, 28),
		color_rgb(57, 31, 33),
		color_rgb(93, 44, 40),
		color_rgb(138, 72, 54),
		color_rgb(191, 111, 74),
		color_rgb(230, 156, 105),
		color_rgb(246, 202, 159),
		color_rgb(249, 230, 207),
		color_rgb(237, 171, 80),
		color_rgb(224, 116, 56),
		color_rgb(198, 69, 36),
		color_rgb(142, 37, 29),
		color_rgb(255, 80, 0),
		color_rgb(237, 118, 20),
		color_rgb(255, 162, 20),
		color_rgb(255, 200, 37),
		color_rgb(255, 235, 87),
		color_rgb(211, 252, 126),
		color_rgb(153, 230, 95),
		color_rgb(90, 197, 79),
		color_rgb(51, 152, 75),
		color_rgb(30, 111, 80),
		color_rgb(19, 76, 76),
		color_rgb(12, 46, 68),
		color_rgb(0, 57, 109),
		color_rgb(0, 105, 170),
		color_rgb(0, 152, 220),
		color_rgb(0, 205, 249),
		color_rgb(12, 241, 255),
		color_rgb(148, 253, 255),
		color_rgb(253, 210, 237),
		color_rgb(243, 137, 245),
		color_rgb(219, 63, 253),
		color_rgb(122, 9, 250),
		color_rgb(48, 3, 217),
		color_rgb(12, 2, 147),
		color_rgb(3, 25, 63),
		color_rgb(59, 20, 67),
		color_rgb(98, 36, 97),
		color_rgb(147, 56, 143),
		color_rgb(202, 82, 201),
		color_rgb(200, 80, 134),
		color_rgb(246, 129, 135),
		color_rgb(245, 85, 93),
		color_rgb(234, 50, 60),
		color_rgb(196, 36, 48),
		color_rgb(137, 30, 43),
		color_rgb(87, 28, 39)
	};

	#pragma endregion

	SurfaceScene::SurfaceScene(Game2* game2) : Scene2(game2)
	{
		auto settings = game->get_settings();
		auto layer = game->get_renderer()->create_composite_layer("main", 1.0f);

		// Set up default camera centered around origin
		auto camera = std::make_unique<Camera2>(game);
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->set_resize_handler(fixed_height_camera(texture_height));
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
		info_text->set_size(10.0f);
		info_text->transform.position = Vector3(-0.45f * static_cast<float>(texture_width), -0.2f * static_cast<float>(texture_height), 0.0f);
		std::stringstream ss;
		ss << "Surface Test" << std::endl
			<< "Press 1: Load test image" << std::endl
			<< "Press 2: B/W test image" << std::endl
			<< "Press 3: Color test image" << std::endl
			<< "Press 4: Random noise" << std::endl
			<< std::endl
			<< "Press B: Blend image" << std::endl
			<< "Press D: Apply dithering" << std::endl
			<< "Press G: Convert to grayscale" << std::endl
			<< "Press H: Flip image horizontally" << std::endl
			<< "Press O: Apply ordered dithering" << std::endl
			<< "Press V: Flip image horizontally" << std::endl
			<< "F11: Hide info text" << std::endl;
		info_text->set_text(ss.str());
		info_text->update();
		info_layer->add(info_text.get());

		// Set up debug layer
		auto debug_layer = game->get_renderer()->create_direct_layer("debug", 1000.0f);
		debug_text = game->create_text_mesh();
		debug_text->set_size(4.0f);
		debug_text->transform.position = Vector3(-0.5f * (float)texture_width, -0.5f * (float)texture_height, 0.0f);
		debug_layer->add(debug_text.get());
		debug_layer->hide();
	
		game->get<TimerManager>()->create(1.0f, [&]() {
			std::stringstream ss;
			auto window = game->get_window();
			auto cam = game->get_renderer()->get_camera();
			ss << "WIN: " << window->get_width() << "x" << window->get_height()
				<< " VIR: " << cam->transform.dimension.x << "x" << cam->transform.dimension.y
				<< " FPS: " << game->get_fps()
				<< " MESH: " << dukat::perfc.avg(dukat::PerformanceCounter::MESHES)
				<< " VERT: " << dukat::perfc.avg(dukat::PerformanceCounter::VERTICES) << std::endl;
			debug_text->set_text(ss.str());
			debug_text->update();
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
			test_load_image("../assets/textures/horse32.png");
			break;
		case SDLK_2:
			test_bw_image();
			break;
		case SDLK_3:
			test_color_image();
			break;
		case SDLK_4:
			test_random_image();
			break;

		case SDLK_b:
			if (surface != nullptr)
			{
				auto img = load_surface("../assets/textures/test_rgb24.png");
				blend(*img, *surface);
				texture->load_data(*surface, TextureFilterProfile::ProfileNearest);
			}
			break;
		case SDLK_d:
			if (surface != nullptr)
				apply_dithering(1);
			break;

		case SDLK_g:
			if (surface != nullptr)
				convert_to_grayscale();
			break;

		case SDLK_h:
			if (surface != nullptr)
			{
				surface->flip_horizontal();
				texture->load_data(*surface, TextureFilterProfile::ProfileNearest);
			}
			break;

		case SDLK_o:
			if (surface != nullptr)
				apply_dithering(3);
			break;

		case SDLK_v:
			if (surface != nullptr)
			{
				surface->flip_vertical();
				texture->load_data(*surface, TextureFilterProfile::ProfileNearest);
			}
			break;

		case SDLK_F11:
			info_text->set_alpha(info_text->get_alpha() == 0.0f ? 1.0f : 0.0f);
			break;
		}
	}
	
	void SurfaceScene::test_load_image(const std::string& filename)
	{
		surface = load_surface(filename);
		if (surface->get_surface()->format->format != SDL_PIXELFORMAT_RGBA8888)
			surface->convert_format(SDL_PIXELFORMAT_RGBA8888);
		texture->load_data(*surface, TextureFilterProfile::ProfileNearest);
	}

	void SurfaceScene::test_bw_image(void)
	{
		surface = std::make_unique<Surface>(texture_width, texture_height, SDL_PIXELFORMAT_RGBA8888);
		constexpr auto grad = static_cast<float>(2 * 1000);
		for (auto x = 0; x < texture_width; x++)
		{
			for (auto y = 0; y < texture_height; y++)
			{
				const auto dist = static_cast<float>((x - texture_width / 2) * (x - texture_width / 2)
					+ (y - texture_height / 2) * (y - texture_height / 2));
				auto alpha = (255.0f * (1.0f - dist / grad));
				surface->set_color(x, y, Color{ 1.0f, 1.0f, 1.0f, alpha });
			}
		}
		texture->load_data(*surface, TextureFilterProfile::ProfileNearest);
	}

	void SurfaceScene::test_color_image(void)
	{
		surface = std::make_unique<Surface>(texture_width, texture_height, SDL_PIXELFORMAT_RGBA8888);
		fill_rect(*surface, 0, 0, 64, 64, Color{ 0.f, 0.f, 0.f, 1.f });
		fill_rect(*surface, 64, 0, 64, 64, Color{ 1.f, 0.f, 0.f, 1.f });
		fill_rect(*surface, 128, 0, 64, 64, Color{ 0.f, 1.f, 0.f, 1.f });
		fill_rect(*surface, 192, 0, 64, 64, Color{ 0.f, 0.f, 1.f, 1.f });
		fill_rect(*surface, 0, 64, 64, 64, Color{ 1.f, 1.f, 0.f, 1.f });
		fill_rect(*surface, 64, 64, 64, 64, Color{ 1.f, 0.f, 1.f, 1.f });
		fill_rect(*surface, 128, 64, 64, 64, Color{ 0.f, 1.f, 1.f, 1.f });
		fill_rect(*surface, 192, 64, 64, 64, Color{ 1.f, 1.f, 1.f, 1.f });
		fill_rect(*surface, 0, 128, 64, 64, Color{ 0.f, 0.f, 0.f, 0.5f });
		fill_rect(*surface, 64, 128, 64, 64, Color{ 1.f, 0.f, 0.f, 0.5f });
		fill_rect(*surface, 128, 128, 64, 64, Color{ 0.f, 1.f, 0.f, 0.5f });
		fill_rect(*surface, 192, 128, 64, 64, Color{ 0.f, 0.f, 1.f, 0.5f });
		fill_rect(*surface, 0, 192, 64, 64, Color{ 1.f, 1.f, 0.f, 0.5f });
		fill_rect(*surface, 64, 192, 64, 64, Color{ 1.f, 0.f, 1.f, 0.5f });
		fill_rect(*surface, 128, 192, 64, 64, Color{ 0.f, 1.f, 1.f, 0.5f });
		fill_rect(*surface, 192, 192, 64, 64, Color{ 1.f, 1.f, 1.f, 0.5f });
		texture->load_data(*surface, TextureFilterProfile::ProfileNearest);
	}

	void SurfaceScene::test_random_image(void)
	{
		surface = std::make_unique<Surface>(texture_width, texture_height, SDL_PIXELFORMAT_RGBA8888);

		rand::Generator<uint8_t> gen;
		for (auto y = 0; y < texture_height; y++)
		{
			for (auto x = 0; x < texture_width; x++)
			{
				const auto color = gen.operator()();
				surface->set_pixel(x, y, (color << 24) | (color << 16) | (color << 8) | 0xff);
			}
		}

		texture->load_data(*surface, TextureFilterProfile::ProfileNearest);
	}

	void SurfaceScene::apply_dithering(int flavor)
	{
		auto src_surface = Surface(*surface);
		surface = std::make_unique<Surface>(texture_width, texture_height, SDL_PIXELFORMAT_RGBA8888);
		switch (flavor) 
		{
		case 1:
			dither_image(src_surface, FloydSteinbergDitherAlgorithm<float,3>{ src_surface.width(), src_surface.height() }, color_palette, *surface);
			break;
		case 2:
			dither_image(src_surface, SierraDitherAlgorithm<float, 3>{ src_surface.width(), src_surface.height() }, color_palette, *surface);
			break;
		case 3:
			dither_image(src_surface, OrderedDitherAlgorithm<float, 3>{ 2 }, color_palette, *surface);
			break;
		}
		texture->load_data(*surface, TextureFilterProfile::ProfileNearest);
	}

	void SurfaceScene::convert_to_grayscale(void)
	{
		for (auto y = 0; y < texture_height; y++)
		{
			for (auto x = 0; x < texture_width; x++)
			{
				const auto src_color = surface->get_color(x, y);
				surface->set_color(x, y, rgb_to_gray(src_color));
			}
		}
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