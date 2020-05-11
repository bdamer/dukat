// flockingapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "flockingapp.h"

namespace dukat
{
	const int num_boids = 1;

	FlockingScene::FlockingScene(Game2* game2) : Scene2(game2)
	{
		auto settings = game->get_settings();
		// Set up default camera centered around origin
		auto camera = std::make_unique<Camera2>(game, Vector2(window_width, window_height));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->transform.position = Vector2{ 0.5f * window_width, 0.5f * window_height };
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		particle_layer = game->get_renderer()->create_direct_layer("particles", 15.0f);
		main_layer = game->get_renderer()->create_direct_layer("main", 20.0f);
		
		// Add cursor
		auto texture = game->get_textures()->get("cursor32.png");
		cursor = std::make_unique<Sprite>(texture);
		cursor->flags = Sprite::align_left | Sprite::align_top | Sprite::relative;
		main_layer->add(cursor.get());

		// Create some boids
		for (auto i = 0; i < num_boids; i++)
			add_boid(random({ 0,0 }, { window_width, window_height }));

		// Event handlers
		game->get_devices()->active->on_press(InputDevice::Button1, [&](void) {
			auto ctrl = game->get_devices()->active;
			add_boid(Vector2{ ctrl->rxa, ctrl->rya });
		});

		game->get_devices()->active->on_press(InputDevice::Button2, [&](void) {
			auto ctrl = game->get_devices()->active;
			add_boid(Vector2{ ctrl->rxa, ctrl->rya }, true);
		});

		// Set up info text
		info_text = game->create_text_mesh();
		info_text->set_size(16.0f);
		info_text->transform.position = Vector3(-0.5f * (float)window_width, 0.4f * (float)window_height, 0.0f);
		info_text->transform.update();
		std::stringstream ss;
		ss << "<Left Button> Add to flock" << std::endl
			<< "<Right Button> Add predator" << std::endl
			<< "<F11> Toggle Info" << std::endl
			<< "<ESC> Quit" << std::endl;
		info_text->set_text(ss.str());
		main_layer->add(info_text.get());

		// Set up debug layer
		auto debug_layer = game->get_renderer()->create_composite_layer("debug", 1000.0f);
		debug_text = game->create_text_mesh();
		debug_text->set_size(16.0f);
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
				<< " PART: " << dukat::perfc.avg(dukat::PerformanceCounter::PARTICLES)
				<< " VERT: " << dukat::perfc.avg(dukat::PerformanceCounter::VERTICES) << std::endl;
			debug_text->set_text(ss.str());
		}, true);

		game->set_controller(this);
	}

	void FlockingScene::add_boid(const Vector2& pos, bool predator)
	{
		auto p = game->get<ParticleManager>()->create_particle();
		p->pos = pos;
		if (predator)
		{
			p->color = { 1.0f, 0.0f, 0.0f, random(0.5f, 1.0f) };
		}
		else
		{
			p->color = { 1.0f, 1.0f, 1.0f, random(0.5f, 1.0f) };
		}
		p->size = 4.0f;
		p->dp = Vector2{ 1.0f, 0.0f };
		p->dc = { 0.0f, 0.0f, 0.0f, 0.0f };
		p->ttl = 1800.0f;
		particle_layer->add(p);
		boids.push_back(Boid{ p, predator });
	}

	void FlockingScene::handle_keyboard(const SDL_Event & e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;
		case SDLK_F11:
			info_text->visible = !info_text->visible;
			break;
		}
	}

	void FlockingScene::update(float delta)
	{
		auto dev = game->get_devices()->active;
		Vector2 input(dev->lx, -dev->ly);

		cursor->p.x = dev->rxa - 0.5f * window_width;
		cursor->p.y = dev->rya - 0.5f * window_height;

		std::for_each(boids.begin(), boids.end(), [&](Boid& b) { b.update(boids);  });
		
		Scene2::update(delta);

		// wrap around
		std::for_each(boids.begin(), boids.end(), [](Boid& b) {
			if (b.p->pos.x < 0.0f)
				b.p->pos.x += window_width;
			else if (b.p->pos.x > window_width)
				b.p->pos.x -= window_width;
			if (b.p->pos.y < 0.0f)
				b.p->pos.y += window_height;
			else if (b.p->pos.y > window_height)
				b.p->pos.y -= window_height;
		});
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/flocking.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game2 app(settings);
		app.add_scene("main", std::make_unique<dukat::FlockingScene>(&app));
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