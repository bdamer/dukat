// particles.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "particles.h"

namespace dukat
{
	ParticlesScene::ParticlesScene(Game2* game2) : Scene2(game2), particle_accu(0.0f)
	{
		auto settings = game->get_settings();
		particle_layer = game->get_renderer()->create_layer("main", 1.0f);

		// Set up default camera centered around origin
		auto camera = std::make_unique<Camera2>(game, Vector2(camera_width, camera_height));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		// Set up info text
		auto info_layer = game->get_renderer()->create_layer("overlay", 25.0f);
		info_layer->stage = RenderStage::OVERLAY;
		info_text = game->create_text_mesh();
		info_text->set_size(8.0f);
		info_text->transform.position = Vector3(-0.5f * static_cast<float>(camera_width), 
			0.4f * static_cast<float>(camera_height), 0.0f);
		info_text->transform.update();
		info_layer->add(info_text.get());

		// Set up debug layer
		auto debug_layer = game->get_renderer()->create_layer("debug", 1000.0f);
		debug_layer->stage = RenderStage::OVERLAY;
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
				<< " MESH: " << perfc.avg(PerformanceCounter::MESHES)
				<< " VERT: " << perfc.avg(PerformanceCounter::VERTICES) 
				<< " PART: " << perfc.avg(PerformanceCounter::PARTICLES)
				<< std::endl;
			debug_text->set_text(ss.str());
		}, true);

		game->set_controller(this);

		// Register particle modes
		modes.push_back(ParticleMode{ "Fire", 
			400.0f, 0.25f, 
			[&](void) {
				emitters.clear();
				auto x_offset = -0.02f;
				for (auto i = 0; i < 9; i++)
				{
					emitters.push_back(Emitter{
						Vector2{ x_offset * static_cast<float>(camera_width), 0.25f * static_cast<float>(camera_height) },
						0.0f, pi_over_two
						});
					x_offset += 0.005f;
				}
			},
			std::bind(&ParticlesScene::update_fire, this, std::placeholders::_1) });

		modes.push_back(ParticleMode{ "Smoke",
			100.0f, 0.15f,
			[&](void) {
				emitters.clear();
				auto x_offset = -0.02f;
				for (auto i = 0; i < 5; i++)
				{
					emitters.push_back(Emitter{
						Vector2{ x_offset * static_cast<float>(camera_width), 0.25f * static_cast<float>(camera_height) },
						0.0f, pi_over_two
						});
					x_offset += 0.01f;
				}
			},
			std::bind(&ParticlesScene::update_smoke, this, std::placeholders::_1) });

		modes.push_back(ParticleMode{ "Fountain",
			200.0f, 0.2f,
			[&](void) {
				emitters.clear();
				auto x_offset = -0.01f;
				for (auto i = 0; i < 5; i++)
				{
					emitters.push_back(Emitter{
						Vector2{ x_offset * static_cast<float>(camera_width), 0.0f * static_cast<float>(camera_height) },
						0.0f, pi_over_two
						});
					x_offset += 0.005f;
				}
			},
			std::bind(&ParticlesScene::update_fountain, this, std::placeholders::_1) });

		modes.push_back(ParticleMode{ "Explosion",
			0.0f, 0.0f,
			[&](void) {
				emitters.clear();
			},
			std::bind(&ParticlesScene::update_explosion, this, std::placeholders::_1) });

		change_particle_mode(0);
	}

	void ParticlesScene::change_particle_mode(int m)
	{
		if (m < 0)
			m = modes.size() - 1;
		else if (m >= static_cast<int>(modes.size()))
			m = 0;

		cur_mode = m;

		auto pm = game->get<ParticleManager>();
		pm->clear();

		std::stringstream ss;
		ss << "Particle mode: " << modes[cur_mode].id << std::endl
		   << "Left: Previous Mode" << std::endl
		   << "Right: Next Mode" << std::endl;
		info_text->set_text(ss.str());

		modes[cur_mode].init();
	}

	void ParticlesScene::update_fire(float delta)
	{
		// FIRE
		// - upward direction
		// - fire should have variable ttl, so that you end up with holes
		// - if using multiple emitters, each emitter should have a current 
		//   direction that swings by random amount; that will cause subsequent 
		//   particles to have similar direction

		auto pm = game->get<ParticleManager>();
		
		static int idx = 0;
		const auto range = 2.0f; // horizontal -range,range
		const auto min_size = 1.0f;
		const auto max_size = 6.0f;

		while (particle_accu >= 1.0f)
		{
			auto& emitter = emitters[++idx % emitters.size()];

			const auto offset = Vector2{ 0.0f, -range }.rotate(emitter.cur_angle);

			auto p = pm->create_particle();
			p->pos = emitter.pos + Vector2{ offset.x, 0.0f };
			p->dp.x = 2.0f * offset.x;
			p->dp.y = randf(-25.0f, -40.0f);

			auto n_size = randf(0.0f, 1.0f);
			p->size = min_size + n_size * (max_size - min_size);

			// the further from center the more red particle starts out as
			auto dist = std::abs(p->pos.x) / 8.0f;
			p->color = { 1.0f, 1.0f - dist * 0.25f, 0.0f, 1.0f };
			p->dc = { 0.0f, -0.5f, 0.0f, -0.05f - n_size * 1.0f };

			// The smaller the particle, the longer it will live
			p->ttl = 1.0f + (1.f - n_size) * 4.0f;

			particle_layer->add(p);

			particle_accu -= 1.0f;
		}
	}

	void ParticlesScene::update_smoke(float delta)
	{
		// SMOKE
		// - upward direction
		// - position of emitter should ocilate on the x axis

		auto pm = game->get<ParticleManager>();

		static int idx = 0;
		const auto range = 4.0f; // horizontal -range,range
		const auto min_size = 4.0f;
		const auto max_size = 8.0f;

		while (particle_accu >= 1.0f)
		{
			auto& emitter = emitters[++idx % emitters.size()];

			const auto offset = Vector2{ 0.0f, -range }.rotate(emitter.cur_angle);

			auto p = pm->create_particle();
			p->pos = emitter.pos + Vector2{ offset.x, 0.0f };
			p->dp.x = offset.x;
			p->dp.y = randf(-15.0f, -25.0f);

			auto n_size = randf(0.0f, 1.0f);
			p->size = min_size + n_size * (max_size - min_size);

			p->color = { 1.0f, 1.0f, 1.0f, 1.0f };
			p->dc = { 0.0f, 0.0f, 0.0f, -0.1f - n_size * 0.5f };

			// The smaller the particle, the longer it will live
			p->ttl = 2.0f + (1.f - n_size) * 4.0f;

			particle_layer->add(p);

			particle_accu -= 1.0f;
		}
	}

	void ParticlesScene::update_fountain(float delta)
	{
		// FOUNTAIN
		// - initial dx, dy
		// - gravity pulls at dy->reduce and ultimately turn around

		auto pm = game->get<ParticleManager>();

		static int idx = 0;
		const auto range = 4.0f; // horizontal -range,range
		const auto min_size = 1.0f;
		const auto max_size = 4.0f;
		std::array<Color, 5> colors = {
			color_rgba(0xffffffcc),
			color_rgba(0x0cf1ffcc),
			color_rgba(0x00cdf9dd),
			color_rgba(0x0098dcee),
			color_rgba(0x0069aaff)
		};

		static float time = 0;

		time += delta;
		const auto max_v = 40.0f + 10.0f * cosf(time);

		while (particle_accu >= 1.0f)
		{
			auto& emitter = emitters[++idx % emitters.size()];

			const auto offset = Vector2{ 0.0f, -range }.rotate(emitter.cur_angle);

			auto p = pm->create_particle();
			p->flags |= Particle::Gravitational;
			p->pos = emitter.pos + Vector2{ offset.x, 0.0f };
			p->dp.x = offset.x;
			p->dp.y = randf(-30.0f, -max_v);
			
			auto size = randi(1, 6);
			p->size = randf(1.0f, 4.0f);// static_cast<float>(size);
			p->color = colors[size - 1];
			p->dc = { 0.0f, 0.0f, 0.0f, -0.05f };
			p->ttl = 4.0f;

			particle_layer->add(p);

			particle_accu -= 1.0f;
		}
	}

	void ParticlesScene::update_explosion(float delta)
	{
		const auto max_time = 5.0f;
		static auto time = max_time;
		time += delta;

		if (time < max_time)
			return;

		time = 0.0f;

		auto pm = game->get<ParticleManager>();
		const auto min_size = 1.0f;
		const auto max_size = 6.0f;

		for (int i = 0; i < 200; i++)
		{
			const auto angle = randf(0.0f, two_pi);
			const auto offset = Vector2{ 0.0f, -1.0f }.rotate(angle);

			auto p = pm->create_particle();
			p->flags |= Particle::Dampened;
			p->pos = Vector2{ 0.0f, 0.0f };
			p->dp = offset * randf(25.0f, 35.0f);

			auto n_size = randf(0.0f, 1.0f);
			p->size = min_size + n_size * (max_size - min_size);

			// the further from center the more red particle starts out as
			auto dist = std::abs(p->pos.x) / 8.0f;
			p->color = { 1.0f, 1.0f - dist * 0.25f, 0.0f, 1.0f };
			p->dc = { 0.0f, -0.5f, 0.0f, -0.1f - n_size * 0.8f };

			// The smaller the particle, the longer it will live
			p->ttl = 1.0f + (1.f - n_size) * 4.0f;

			particle_layer->add(p);
		}
	}

	void ParticlesScene::update(float delta)
	{
		const auto& mode = modes[cur_mode];

		// Update angle of each emitter
		for (auto& e : emitters)
			e.cur_angle += randf(-mode.max_change, mode.max_change);

		// Update accumulator based on particle rate
		particle_accu += delta * mode.particle_rate;

		// Emit new particles
		mode.update(delta);
	}

	void ParticlesScene::handle_keyboard(const SDL_Event& e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;
		case SDLK_LEFT:
			change_particle_mode(cur_mode - 1);
			break;
		case SDLK_RIGHT:
			change_particle_mode(cur_mode + 1);
			break;
		}
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/particles.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game2 app(settings);
		app.add_scene("main", std::make_unique<dukat::ParticlesScene>(&app));
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