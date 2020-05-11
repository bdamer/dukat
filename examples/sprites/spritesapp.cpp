// spritesapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "spritesapp.h"

namespace dukat
{
	SpritesScene::SpritesScene(Game2* game2) : Scene2(game2), particles_enabled(true)
	{
		auto settings = game->get_settings();
		// Set up default camera centered around origin
		auto camera = std::make_unique<Camera2>(game, Vector2(window_width, window_height));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		bg_layer = game->get_renderer()->create_composite_layer("background", 10.0f, 2.0f);
		particle_layer = game->get_renderer()->create_composite_layer("particles", 15.0f);
		main_layer = game->get_renderer()->create_composite_layer("main", 20.0f);

		auto texture = game->get_textures()->get("dukat.png");
		sprite = std::make_unique<Sprite>(texture);
		main_layer->add(sprite.get());

		// Set up info text
		auto info_layer = game->get_renderer()->create_direct_layer("overlay", 25.0f);
		info_text = game->create_text_mesh();
		info_text->set_size(8.0f);
		info_text->transform.position = Vector3(-0.5f * (float)window_width, 0.4f * (float)window_height, 0.0f);
		info_text->transform.update();
		std::stringstream ss;
		ss << "Sprite Test" << std::endl
			<< "<WASD> to move sprite" << std::endl
			<< "<T>oggle particles" << std::endl;
		info_text->set_text(ss.str());
		info_layer->add(info_text.get());

		// Play some tunes
		auto music = game->get_samples()->get_music("space_ambience.mp3");
		game->get_audio()->play_music(music, -1);

		// Set up debug layer
		auto debug_layer = game->get_renderer()->create_direct_layer("debug", 1000.0f);
		debug_text = game->create_text_mesh();
		debug_text->set_size(4.0f);
		debug_text->transform.position = Vector3(-0.5f * (float)window_width, -0.5f * (float)window_height, 0.0f);
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

	void SpritesScene::handle_keyboard(const SDL_Event & e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;
		case SDLK_t:
			particles_enabled = !particles_enabled;
			break;
		}
	}

	void SpritesScene::update(float delta)
	{
		auto dev = game->get_devices()->active;
		Vector2 input(dev->lx, -dev->ly);
		// reduce slightly and add user input
		sprite_vel = sprite_vel * 0.95f + input * 25.0f;
		// Update sprite position
		sprite->p += sprite_vel * delta;

		const auto half_width = 0.5f * static_cast<float>(window_width);
		const auto half_height = 0.5f * static_cast<float>(window_height);
		bool bounce = false;
		if (sprite->p.x < -half_width || sprite->p.x > half_width)
		{
			sprite_vel.x = -sprite_vel.x;
			sprite->p.x = std::min(half_width, std::max(-half_width, sprite->p.x));
			bounce = true;
		}
		if (sprite->p.y < -half_height || sprite->p.y > half_height)
		{
			sprite_vel.y = -sprite_vel.y;
			sprite->p.y = std::min(half_height, std::max(-half_height, sprite->p.y));
			bounce = true;
		}

		audio_delay -= delta;
		if (bounce && audio_delay <= 0.0f)
		{
			game->get_audio()->play_sample(game->get_samples()->get_sample("kickstep.mp3"));
			audio_delay = 0.2f;
		}

		const auto pos_offset = 5.0f;
		const auto vel_offset = 5.0f;

		// Create a new particle
		if (particles_enabled && (input.x != 0.0f || input.y != 0.0f))
		{
			auto p = game->get<ParticleManager>()->create_particle();
			p->pos = sprite->p + Vector2{ random(-pos_offset, pos_offset), random(-pos_offset, pos_offset) }; 
			p->color = { std::abs(input.x), std::abs(input.y), 1.0f, 1.0f };
			p->size = random(5.0f, 10.0f);
			p->dp = input * -15.0f + Vector2{ random(-vel_offset, vel_offset), random(-vel_offset, vel_offset) }; 
			p->dc = { 0.0f, 0.0f, 0.0f, -0.2f };
			p->ttl = 5.0f;
			particle_layer->add(p);
		}

		Scene2::update(delta);
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/sprites.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game2 app(settings);
		app.add_scene("main", std::make_unique<dukat::SpritesScene>(&app));
		app.push_scene("main");
		return app.run();
	}
	catch (const std::exception& e)
	{
		dukat::log->error("Application failed with error: {}", e.what());
		return -1;
	}
	return 0;
}