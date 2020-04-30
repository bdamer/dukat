// layersapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "layersapp.h"

namespace dukat
{
	LayersScene::LayersScene(Game2* game2) : Scene2(game2)
	{
		auto settings = game->get_settings();

		// Set up default camera centered around origin
		auto game_height = static_cast<int>(game_width / game->get_window()->get_aspect_ratio());
		log->debug("Setting virtual resolution to: {}x{}", game_width, game_height);
		auto camera = std::make_unique<Camera2>(game, Vector2(game_width, game_height));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		// Mirror mask 
		mask_texture = game->get_textures()->get("mask2.png");
		// Ground tiles
		ground_texture = game->get_textures()->get("ground32.png");

		// Set up layers
		auto ground_layer = game->get_renderer()->create_layer("ground", 0.0f);
		ground_layer->set_composite_program(game->get_shaders()->get_program("fx_default.vsh", "fx_ground.fsh"));
		ground_layer->set_composite_binder([&](ShaderProgram* p) {
			ground_texture->bind(1, p);
			p->set("u_scale", 8.0f);
		});

		auto bg_layer = game->get_renderer()->create_layer("background", 10.0f);

		auto scene_mirror = game->get_renderer()->create_layer("scene_mirror", 15.0f);
		scene_mirror->set_composite_program(game->get_shaders()->get_program("fx_default.vsh", "fx_mirror.fsh"));
		scene_mirror->set_composite_binder([&](ShaderProgram* p) {
			mask_texture->bind(1, p);
		});
		auto sp = game->get_shaders()->get_program("sc_sprite_mirror.vsh", "sc_sprite.fsh");
		auto pp = game->get_shaders()->get_program("sc_particle_mirror.vsh", "sc_particle.fsh");
		scene_mirror->add(std::make_unique<MirrorEffect2>(sp, pp, "scene"));

		auto scene_layer = game->get_renderer()->create_layer("scene", 20.0f);
		auto ssp = game->get_shaders()->get_program("sc_shadow.vsh", "sc_shadow.fsh");
		scene_layer->add(std::make_unique<ShadowEffect2>(ssp));

		// Load sprites
		water_sprite = std::make_unique<Sprite>(game->get_textures()->get("white.png"));
		water_sprite->w = 60;
		water_sprite->h = 60;
		water_sprite->z = 20;
		water_sprite->color = color_rgb(0x3484b0);
		bg_layer->add(water_sprite.get());

		player_sprite = std::make_unique<Sprite>(game->get_textures()->get("lobber.png"));
		player_sprite->flags |= Sprite::align_bottom;
		scene_layer->add(player_sprite.get());

		barrel_sprite = std::make_unique<Sprite>(game->get_textures()->get("barrel16.png"));
		barrel_sprite->flags |= Sprite::align_bottom;
		scene_layer->add(barrel_sprite.get());

		// Set up info text
		auto info_layer = game->get_renderer()->create_overlay_layer("overlay", 25.0f);
		info_text = game->create_text_mesh();
		info_text->set_size(2.0f);
		info_text->transform.position = Vector3(-0.5f * (float)game_width, 0.0f, 0.0f);
		info_text->transform.update();
		std::stringstream ss;
		ss << "Layers Example" << std::endl
			<< "<W,A,S,D> Movement" << std::endl;
		info_text->set_text(ss.str());
		info_layer->add(info_text.get());
		info_layer->hide();

		// Set up debug layer
		auto debug_layer = game->get_renderer()->create_overlay_layer("debug", 1000.0f);
		debug_text = game->create_text_mesh();
		debug_text->set_size(4.0f);
		debug_text->transform.position = Vector3(-0.5f * (float)game_width, -0.5f * (float)game_height, 0.0f);
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

	void LayersScene::handle_keyboard(const SDL_Event& e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;
		}
	}

	void LayersScene::update(float delta)
	{
		auto dev = game->get_devices()->active;
		player_sprite->p += Vector2{ dev->lx, -dev->ly } * 32.0f * delta;
		// update rendering priority based on sprite's Y position
		player_sprite->z = player_sprite->p.y;
		barrel_sprite->z = barrel_sprite->p.y;

		// spawn particles
		auto pm = game->get<ParticleManager>();
		auto p = pm->create_particle();
		p->pos = Vector2{ random(-4.0f, 4.0f), -random(32.0f, 34.0f) };
		p->ry = -32.0f;
		p->dp = Vector2{ random(-4.0f, 4.0f), -random(12.0f, 16.0f) };
		p->color = Color{ 1.0f, 1.0f, 1.0f, random(0.75f, 1.0f) };
		p->dc = Color{ 0.0f, 0.0f, 0.0f, -0.1f };
		p->ttl = 1.0f;
		game->get_renderer()->get_layer("scene")->add(p);

		Scene2::update(delta);
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/layers.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game2 app(settings);
		app.add_scene("main", std::make_unique<dukat::LayersScene>(&app));
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