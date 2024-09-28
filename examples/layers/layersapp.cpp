// layersapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "layersapp.h"
#include <dukat/causticseffect2.h>

namespace dukat
{
	LayersScene::LayersScene(Game2* game2) : Scene2(game2)
	{
		auto settings = game->get_settings();

		auto window = game->get_window();
		resize(window->get_width(), window->get_height());
		const auto game_height = static_cast<int>(game->get_renderer()->get_camera()->transform.dimension.y);

		MeshBuilder2 mb2;
		if (!game->get_meshes()->contains("quad"))
			game->get_meshes()->put("quad", mb2.build_textured_quad());

		// Mirror mask 
		mask_texture = game->get_textures()->get_or_load("mask1.png");
		// Ground tiles
		ground_texture = game->get_textures()->get_or_load("ground32.png");

		// Set up layers
		auto ground_layer = game->get_renderer()->create_composite_layer("ground", 0.0f);

		auto caustics_sp = game->get_shaders()->get_program("fx_default.vsh", "fx_caustics.fsh");
		auto caustics_fx = std::make_unique<CausticsEffect2>(game, caustics_sp, game_width, game_height, 0.005f);
		const auto fx_ptr = caustics_fx.get();
		ground_layer->add(std::move(caustics_fx));

		ground_layer->set_composite_program(game->get_shaders()->get_program("fx_default.vsh", "fx_layers.fsh"));
		ground_layer->set_composite_binder([&, fx_ptr](ShaderProgram* p) {
			ground_texture->bind(1, p);
			mask_texture->bind(2, p);
			fx_ptr->get_texture()->bind(3, p);
			p->set("u_scale", 8.0f);
			p->set("u_color_lo", 0.051f, 0.09f, 0.286f, 1.f);	// #0d1749
			p->set("u_color_mid", 0.047f, 0.08f, 0.576f, 1.f);	// #0c0293
			p->set("u_color_hi", 0.188f, 0.012f, 0.851f, 1.f);	// #3003d9
		});

		auto bg_layer = game->get_renderer()->create_composite_layer("background", 10.0f);

		auto scene_mirror = game->get_renderer()->create_composite_layer("scene_mirror", 15.0f);
		scene_mirror->set_composite_program(game->get_shaders()->get_program("fx_default.vsh", "fx_mirror.fsh"));
		scene_mirror->set_composite_binder([&](ShaderProgram* p) {
			mask_texture->bind(1, p);
		});
		auto sp = game->get_shaders()->get_program("sc_sprite_mirror.vsh", "sc_sprite.fsh");
		auto pp = game->get_shaders()->get_program("sc_particle_mirror.vsh", "sc_particle.fsh");
		scene_mirror->add(std::make_unique<MirrorEffect2>(sp, pp, "scene"));

		auto scene_shadow = game->get_renderer()->create_composite_layer("scene_shadow", 19.0f);
		auto ssp = game->get_shaders()->get_program("sc_shadow.vsh", "sc_shadow.fsh");
		auto fx = std::make_unique<ShadowEffect2>(ssp, "scene");
		fx->set_radius(0.35f);
		fx->set_alpha(0.3f);
		scene_shadow->add(std::move(fx));

		auto scene_layer = game->get_renderer()->create_composite_layer("scene", 20.0f);

		// Load sprites
		player_sprite = std::make_unique<Sprite>(game->get_textures()->get_or_load("lobber.png"));
		player_sprite->flags |= Sprite::align_bottom;
		scene_layer->add(player_sprite.get());

		barrel_sprite = std::make_unique<Sprite>(game->get_textures()->get_or_load("barrel16.png"));
		barrel_sprite->flags |= Sprite::align_bottom;
		barrel_sprite->p.x = 20.0f;
		barrel_sprite->p.y = -10.0f;
		scene_layer->add(barrel_sprite.get());

		// Set up info text
		auto info_layer = game->get_renderer()->create_direct_layer("overlay", 25.0f);
		info_text = game->create_text_mesh();
		info_text->set_size(2.0f);
		info_text->transform.position = Vector3(-0.5f * (float)game_width, 0.45f * (float)game_height, 0.0f);
		std::stringstream ss;
		ss << "Layers Example" << std::endl
			<< "<W,A,S,D> Movement" << std::endl;
		info_text->set_text(ss.str());
		info_text->update();
		info_layer->add(info_text.get());

		// Set up debug layer
		auto debug_layer = game->get_renderer()->create_direct_layer("debug", 1000.0f);
		debug_text = game->create_text_mesh();
		debug_text->set_size(2.0f);
		debug_text->transform.position = Vector3(-0.5f * (float)game_width, -0.5f * (float)game_height, 0.0f);
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
		p->flags |= Particle::Gravitational;
		p->pos = barrel_sprite->p + Vector2{ random(-4.0f, 4.0f), -random(12.0f, 14.0f) };
		p->ry = barrel_sprite->p.y;
		p->dp = Vector2{ random(-4.0f, 4.0f), -random(12.0f, 16.0f) };
		p->color = Color{ 0.047f, 0.08f, 0.576f, random(0.75f, 1.0f) };
		p->dc = Color{ 0.0f, 0.0f, 0.0f, -0.1f };
		p->ttl = 2.0f;
		game->get_renderer()->get_layer("scene")->add(p);

		Scene2::update(delta);
	}
	
	void LayersScene::resize(int width, int height)
	{
		auto settings = game->get_settings();
		// Set up default camera centered around origin
		auto camera = std::make_unique<Camera2>(game);
		camera->set_clip(settings.get_float(settings::video_camera_nearclip), settings.get_float(settings::video_camera_farclip));
		camera->set_resize_handler(fixed_width_camera(game_width));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));
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
}