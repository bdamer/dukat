// particles.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "particles.h"

namespace dukat
{
	ParticlesScene::ParticlesScene(Game2* game2) : Scene2(game2)
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
		modes.push_back(ParticleMode{ "Linear", 
			[&](void) {
				auto pm = game->get<ParticleManager>();

				ParticleEmitter::Recipe recipe{ 
					ParticleEmitter::Recipe::Linear, 1.0f, 4.0f, 1.0f, 5.0f,
					Vector2{ -15, -15 }, Vector2{ 15, 15 }, 	
					{ 
						Color{ 1.f, 0.f, 0.f, 1.f },
						Color{ 0.f, 1.f, 0.f, 1.f },
						Color{ 0.f, 0.f, 1.f, 1.f },
						Color{ 1.f, 0.f, 1.f, 1.f }
					},
					Color{ 0.f, 0.f, 0.f, -0.1f }
				};

				auto e = pm->create_emitter(recipe);
				e->pos = Vector2{0.f, 0.f };
				e->rate = 200.0f;
				e->target_layer = particle_layer;
			}
		});

		modes.push_back(ParticleMode{ "Flame", 
			[&](void) {
				auto pm = game->get<ParticleManager>();
				auto e = pm->create_emitter(ParticleEmitter::Recipe::FlameRecipe);
				e->pos = Vector2{0.f, 0.25f * static_cast<float>(camera_height) };
				e->rate = 400.0f;
				e->target_layer = particle_layer;
				auto x_offset = -0.02f;
				for (auto i = 0; i < 9; i++)
				{
					e->offsets.push_back(Vector2{ x_offset * static_cast<float>(camera_width), 0.0f });
					x_offset += 0.005f;
				}
			}
		});

		modes.push_back(ParticleMode{ "Smoke",
			[&](void) {
				auto pm = game->get<ParticleManager>();
				auto e = pm->create_emitter(ParticleEmitter::Recipe::SmokeRecipe);
				e->pos = Vector2{ 0.0f, 0.25f * static_cast<float>(camera_height) };
				e->rate = 100.0f;
				e->target_layer = particle_layer;
				auto x_offset = -0.02f;
				for (auto i = 0; i < 5; i++)
				{
					e->offsets.push_back(Vector2{ x_offset * static_cast<float>(camera_width), 0.0f });
					x_offset += 0.01f;
				}
			}
		});

		modes.push_back(ParticleMode{ "Fountain",
			[&](void) {
				auto pm = game->get<ParticleManager>();
				auto e = pm->create_emitter(ParticleEmitter::Recipe::FountainRecipe);
				e->pos = Vector2{ 0.f, 0.f };
				e->rate = 200.0f;
				e->target_layer = particle_layer;
				auto x_offset = -0.01f;
				for (auto i = 0; i < 5; i++)
				{
					e->offsets.push_back(Vector2{ x_offset * static_cast<float>(camera_width), 0.0f });
					x_offset += 0.005f;
				}
			}
		});

		modes.push_back(ParticleMode{ "Explosion",
			[&](void) {
				auto pm = game->get<ParticleManager>();
				auto e = pm->create_emitter(ParticleEmitter::Recipe::ExplosionRecipe);
				e->pos = Vector2{ 0.0f, 0.0f };
				e->rate = 100.0f;
				e->target_layer = particle_layer;
			}
		});

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