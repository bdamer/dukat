// spritesapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "spritesapp.h"

namespace dukat
{
	void Game::init(void)
	{
		Game2::init();

		// Set up default camera centered around origin
		auto camera = std::make_unique<Camera2>(window.get(), Vector2(window_width, window_height));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		renderer->set_camera(std::move(camera));

		bg_layer = renderer->create_layer("background", 10.0f, 2.0f);
		particle_layer = renderer->create_layer("particles", 15.0f);
		main_layer = renderer->create_layer("main", 20.0f);
		
		auto texture = texture_cache->get("dukat.png");
		sprite = std::make_unique<Sprite>(texture);
		main_layer->add(sprite.get());

		// Set up info text
		info_text = create_text_mesh(8.0f);
		info_text->transform.position = Vector3(-0.5f * (float)window_width, 0.4f * (float)window_height, 0.0f);
		info_text->transform.update();
		std::stringstream ss;
		ss << "Sprite Test" << std::endl
			<< "<WASD> to move sprite" << std::endl
			<< "<T>oggle particles" << std::endl;
		info_text->set_text(ss.str());
		main_layer->add(info_text.get());

		// Set up debug layer
		auto debug_layer = renderer->create_layer("debug", 1000.0f);
		debug_text = create_text_mesh(4.0f);
		debug_text->transform.position = Vector3(-0.5f * (float)window_width, -0.5f * (float)window_height, 0.0f);
		debug_text->transform.update();
		debug_layer->add(debug_text.get());
		debug_layer->hide();
	}

	void Game::handle_keyboard(const SDL_Event & e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_t:
			particles_enabled = !particles_enabled;
			break;
		default:
			Game2::handle_keyboard(e);
		}
	}

	void Game::update(float delta)
	{
		Vector2 input(device_manager->active->lx, -device_manager->active->ly);
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
			auto p = particle_manager->create_particle();
			p->pos = sprite->p + Vector2{ randf(-pos_offset, pos_offset), randf(-pos_offset, pos_offset) }; 
			p->color = { std::abs(input.x), std::abs(input.y), 1.0f, 1.0f };
			p->size = randf(5.0f, 10.0f);
			p->dp = input * -15.0f + Vector2{ randf(-vel_offset, vel_offset), randf(-vel_offset, vel_offset) }; 
			p->dc = { 0.0f, 0.0f, 0.0f, -0.2f };
			p->ttl = 5.0f;
			particle_layer->add(p);
		}

		Game2::update(delta);
	}

	void Game::update_debug_text(void)
	{
		std::stringstream ss;
		auto cam = renderer->get_camera();
		ss << "WIN: " << window->get_width() << "x" << window->get_height()
			<< " VIR: " << cam->transform.dimension.x << "x" << cam->transform.dimension.y
			<< " FPS: " << get_fps()
			<< " MESH: " << dukat::perfc.avg(dukat::PerformanceCounter::MESHES)
			<< " VERT: " << dukat::perfc.avg(dukat::PerformanceCounter::VERTICES) << std::endl;
		debug_text->set_text(ss.str());
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