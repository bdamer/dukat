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
		auto camera = std::make_unique<Camera2>(game->get_window(), Vector2(window_width, window_height));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		bg_layer = game->get_renderer()->create_layer("background", 10.0f, 2.0f);
		particle_layer = game->get_renderer()->create_layer("particles", 15.0f);
		main_layer = game->get_renderer()->create_layer("main", 20.0f);
		
		auto texture = game->get_textures()->get("dukat.png");
		sprite = std::make_unique<Sprite>(texture);
		main_layer->add(sprite.get());

		// Set up info text
		info_text = game->create_text_mesh(8.0f);
		info_text->transform.position = Vector3(-0.5f * (float)window_width, 0.4f * (float)window_height, 0.0f);
		info_text->transform.update();
		std::stringstream ss;
		ss << "Sprite Test" << std::endl
			<< "<WASD> to move sprite" << std::endl
			<< "<T>oggle particles" << std::endl;
		info_text->set_text(ss.str());
		main_layer->add(info_text.get());

		// Set up debug layer
		auto debug_layer = game->get_renderer()->create_layer("debug", 1000.0f);
		debug_text = game->create_text_mesh(4.0f);
		debug_text->transform.position = Vector3(-0.5f * (float)window_width, -0.5f * (float)window_height, 0.0f);
		debug_text->transform.update();
		debug_layer->add(debug_text.get());
		debug_layer->hide();

		game->get_timers()->create_timer(1.0f, [&]() {
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

	bool SpritesScene::handle_keyboard(const SDL_Event & e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_t:
			particles_enabled = !particles_enabled;
			break;
		default:
			return false;
		}
		return true;
	}

	void SpritesScene::update(float delta)
	{
		auto dev = game->get_devices()->active;
		Vector2 input(dev->lx, -dev->ly);
		// reduce slightly and add user input
		sprite_vel = sprite_vel * 0.95f + input * 25.0f;
		// Update sprite position
		sprite->p += sprite_vel * delta;

		const auto half_width = window_width / 2;
		const auto half_height = window_height / 2;
		if (sprite->p.x < -half_width || sprite->p.x > half_width)
		{
			sprite_vel.x = -sprite_vel.x;
		}
		if (sprite->p.y < -half_height || sprite->p.y > half_height)
		{
			sprite_vel.y = -sprite_vel.y;
		}

		const float pos_offset = 5.0f;
		const float vel_offset = 5.0f;

		// Create a new particle
		if (particles_enabled && (input.x != 0.0f || input.y != 0.0f))
		{
			auto p = game->get_particles()->create_particle();
			p->pos = sprite->p + Vector2{ randf(-pos_offset, pos_offset), randf(-pos_offset, pos_offset) }; 
			p->color = { std::abs(input.x), std::abs(input.y), 1.0f, 1.0f };
			p->size = randf(5.0f, 10.0f);
			p->dp = input * -15.0f + Vector2{ randf(-vel_offset, vel_offset), randf(-vel_offset, vel_offset) }; 
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
		dukat::logger << "Application failed with error." << std::endl << e.what() << std::endl;
		return -1;
	}
	return 0;
}