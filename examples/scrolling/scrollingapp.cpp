// scrollingapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "scrollingapp.h"

namespace dukat
{
	ScrollingScene::ScrollingScene(Game2* game2) : Scene2(game2), world_x(0)
	{
		// Set up default camera centered around origin
		player = std::make_unique<Player>();
		switch_to_delayed_camera();

		// Set up layers
		auto bg_layer = game->get_renderer()->create_composite_layer("background", 10.0f);
		auto scene_layer = game->get_renderer()->create_composite_layer("scene", 20.0f);
		auto ui_layer = game->get_renderer()->create_direct_layer("ui", 40.0f);

		// Load sprites
		init_background(bg_layer);

		heart_sprite = std::make_unique<Sprite>(game->get_textures()->get_or_load("scroll_tiles.png"), Rect{ 48, 32, tile_size, tile_size });
		heart_sprite->flags |= Sprite::align_bottom | Sprite::relative;
		heart_sprite->p = Vector2{ -3 * scaled_tile_size, -3 * scaled_tile_size };
		heart_sprite->scale = tile_scale;
		ui_layer->add(heart_sprite.get());

		star_sprite = std::make_unique<Sprite>(game->get_textures()->get_or_load("scroll_tiles.png"), Rect{ 48, 48, tile_size, tile_size });
		star_sprite->flags |= Sprite::align_bottom | Sprite::relative;
		star_sprite->p = Vector2{ 4 * scaled_tile_size, 0 };
		star_sprite->scale = tile_scale;
		ui_layer->add(star_sprite.get());

		player_sprite = std::make_unique<Sprite>(game->get_textures()->get_or_load("lobber.png"));
		player_sprite->flags |= Sprite::align_bottom | Sprite::flip_h;
		player_sprite->scale = tile_scale;
		scene_layer->add(player_sprite.get());

		// Set up info text
		auto info_layer = game->get_renderer()->create_direct_layer("overlay", 25.0f);
		info_text = game->create_text_mesh();
		info_text->set_size(8.0f);
		info_text->transform.position = Vector3(-0.45f * (float)game_width, 0.20f * (float)game_height, 0.0f);
		std::stringstream ss;
		ss << "Scrolling Example" << std::endl
			<< "<W,A,S,D> Movement" << std::endl
			<< "<F2> Fixed camera" << std::endl
			<< "<F3> Follower camera" << std::endl
			<< "<F4> Follower camera (delayed)";
		info_text->set_text(ss.str());
		info_text->update();
		info_layer->add(info_text.get());

		// Set up debug layer
		auto debug_layer = game->get_renderer()->create_direct_layer("debug", 1000.0f);
		debug_text = game->create_text_mesh();
		debug_text->set_size(4 * tile_scale);
		debug_text->transform.position = Vector3(-0.5f * (float)game_width, -0.5f * (float)game_height, 0.0f);
		debug_layer->add(debug_text.get());
		debug_layer->hide();
	
		game->get<TimerManager>()->create(1.0f, [&]() {
			std::stringstream ss;
			auto window = game->get_window();
			auto cam = game->get_renderer()->get_camera();
			ss << "WIN: " << window->get_width() << "x" << window->get_height()
				<< (window->is_fullscreen() ? " (FULL)" : " (WINDOW)")
				<< " VIR: " << cam->transform.dimension.x << "x" << cam->transform.dimension.y
				<< " FPS: " << game->get_fps()
				<< " MESH: " << dukat::perfc.avg(dukat::PerformanceCounter::MESHES)
				<< " VERT: " << dukat::perfc.avg(dukat::PerformanceCounter::VERTICES)
				<< " CAM: " << cam->transform.position.x << "," << cam->transform.position.y
				<< std::endl;
				debug_text->set_text(ss.str());
				debug_text->update();
		}, true);

		game->set_controller(this);
	}

	void ScrollingScene::init_background(RenderLayer2* layer)
	{
		// Ground Sprites
		background_sprites.resize(tile_rows * tile_cols);
		Texture* t = game->get_textures()->get_or_load("scroll_tiles.png");

		int idx = 0;
		// Two rows of dark blue
		init_row(idx++, t, Rect{ 16, 0, tile_size, tile_size }, layer);
		init_row(idx++, t, Rect{ 16, 0, tile_size, tile_size }, layer);
		init_row(idx++, t, Rect{ 16, 0, tile_size, tile_size }, layer);
		// Transition
		init_row(idx++, t, Rect{ 16, 16, tile_size, tile_size }, layer);
		// Row of medium blue
		init_row(idx++, t, Rect{ 16, 32, tile_size, tile_size }, layer);
		// Transition
		init_row(idx++, t, Rect{ 16, 48, tile_size, tile_size }, layer);
		// Row of light blue
		init_row(idx++, t, Rect{ 0, 0, tile_size, tile_size }, layer);
		// Two rows of ground
		init_row(idx++, t, Rect{ 0, 16, tile_size, tile_size }, layer);
		init_row(idx++, t, Rect{ 0, 32, tile_size, tile_size }, layer);
		// Five rows of bedrock
		init_row(idx++, t, Rect{ 0, 48, tile_size, tile_size }, layer);
		init_row(idx++, t, Rect{ 0, 48, tile_size, tile_size }, layer);
		init_row(idx++, t, Rect{ 0, 48, tile_size, tile_size }, layer);
		init_row(idx++, t, Rect{ 0, 48, tile_size, tile_size }, layer);
		init_row(idx++, t, Rect{ 0, 48, tile_size, tile_size }, layer);
	}

	void ScrollingScene::init_row(int index, Texture* texture, const Rect& rect, RenderLayer2* layer)
	{
		auto offset = index * tile_cols;
		for (auto i = 0; i < tile_cols; i++)
		{
			background_sprites[offset + i] = std::make_unique<Sprite>(texture, rect);
			background_sprites[offset + i]->flags |= Sprite::align_bottom | Sprite::align_left;
			background_sprites[offset + i]->p.x = static_cast<float>(i * scaled_tile_size);
			background_sprites[offset + i]->p.y = static_cast<float>((-tile_rows / 2 + index) * scaled_tile_size);
			background_sprites[offset + i]->scale = tile_scale;
			layer->add(background_sprites[offset + i].get());
		}
	}

	void ScrollingScene::update(float delta)
	{
		// Update player based on user input
		auto dev = game->get_devices()->active;
		auto dir = Vector2{ dev->lx, -dev->ly };
		player->pos += dir * 64.0f * delta * static_cast<float>(tile_scale);
		// Update player sprite
		player_sprite->p = player->pos;

		// Rotate star sprite
		star_sprite->rot += delta;

		Scene2::update(delta);

		update_tiles();
	}

	void ScrollingScene::update_tiles(void)
	{
		// figure out x offset
		auto cam = game->get_renderer()->get_camera();
		const auto x = static_cast<int>(cam->transform.position.x);
		const auto offset = 1 + (x / scaled_tile_size) - tile_cols / 2;
		if (world_x != offset) // world offset needs to be updated
		{
			world_x = offset;

			// Shift rows based on updated world x offset
			for (auto index = 0; index < tile_rows; index++)
			{
				auto offset = index * tile_cols;
				for (auto i = 0; i < tile_cols; i++)
				{
					background_sprites[offset + i]->p.x = static_cast<float>((world_x + i) * scaled_tile_size);
				}
			}
		}
	}

	void ScrollingScene::switch_to_fixed_camera(void)
	{
		const auto& settings = game->get_settings();

		auto camera = std::make_unique<Camera2>(game);
		camera->set_clip(settings.get_float(settings::video_camera_nearclip), settings.get_float(settings::video_camera_farclip));
		camera->set_resize_handler(fixed_width_camera(game_width));
		camera->refresh();
		game_height = static_cast<int>(camera->transform.dimension.y);
		game->get_renderer()->set_camera(std::move(camera));
	}

	void ScrollingScene::switch_to_follower_camera(void)
	{
		const auto& settings = game->get_settings();
		game_width = settings.get_int(settings::game_width, 320);

		auto camera = std::make_unique<FollowerCamera2<Player>>(game);
		camera->set_clip(settings.get_float(settings::video_camera_nearclip), settings.get_float(settings::video_camera_farclip));
		camera->set_resize_handler(fixed_width_camera(game_width));
		camera->set_target(player.get());
		camera->refresh();
		game_height = static_cast<int>(camera->transform.dimension.y);
		game->get_renderer()->set_camera(std::move(camera));
	}

	void ScrollingScene::switch_to_delayed_camera(void)
	{
		const auto& settings = game->get_settings();
		game_width = settings.get_int(settings::game_width, 320);

		auto camera = std::make_unique<FollowerCamera2<Player>>(game);
		camera->set_sharpness(0.2f);
		camera->set_clip(settings.get_float(settings::video_camera_nearclip), settings.get_float(settings::video_camera_farclip));
		camera->set_resize_handler(fixed_width_camera(game_width));
		camera->set_target(player.get());
		camera->refresh();
		game_height = static_cast<int>(camera->transform.dimension.y);
		game->get_renderer()->set_camera(std::move(camera));
	}

	void ScrollingScene::handle_keyboard(const SDL_Event& e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_F2:
			switch_to_fixed_camera();
			break;
		case SDLK_F3:
			switch_to_follower_camera();
			break;
		case SDLK_F4:
			switch_to_delayed_camera();
			break;

		case SDLK_ESCAPE:
			game->set_done(true);
			break;
		}
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/scrolling.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game2 app(settings);
		app.add_scene("main", std::make_unique<dukat::ScrollingScene>(&app));
		app.push_scene("main");
		return app.run();
	}
	catch (const std::exception& e)
	{
		dukat::log->error("Application failed with error: {}", e.what());
		return -1;
	}
}