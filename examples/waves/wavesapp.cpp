// wavesapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "wavesapp.h"

namespace dukat
{
	constexpr int WaveScene::grid_size;

	WaveScene::WaveScene(Game3* game) : game(game)
	{
		MeshBuilder3 mb3;

		glClearColor(0.0f, 0.1f, 0.5f, 1.0f );

		auto settings = game->get_settings();

		// Create scene objects
		auto camera = std::make_unique<OrbitCamera3>(game, camera_target, 50.0f, 0.0f, pi_over_four);
		camera->set_min_distance(5.0f);
		camera->set_max_distance(100.0f);
		camera->set_vertical_fov(settings.get_float("camera.fov"));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));		

		object_meshes.stage = RenderStage::SCENE;
		object_meshes.visible = true;

		// load environment map
		auto env_map = game->get_textures()->get("sorsea.dds");

		// Wave mesh
		wave_mesh = std::make_unique<WaveMesh>(game, grid_size);
		wave_mesh->tile_spacing = 4.0f;
		wave_mesh->set_env_map(env_map);

		// Skydome
		skydome_mesh = object_meshes.create_instance();
		skydome_mesh->set_mesh(game->get_meshes()->put("skydome", mb3.build_dome(32, 24, true)));
		skydome_mesh->set_program(game->get_shaders()->get_program("sc_skydome.vsh", "sc_skydome.fsh"));
		skydome_mesh->set_texture(env_map, 0);

		init_environment();
		
		overlay_meshes.stage = RenderStage::OVERLAY;
		overlay_meshes.visible = true;

		// Quad used to visualize normal map
		MeshBuilder2 mb2;
		quad_mesh = overlay_meshes.create_instance();
		quad_mesh->set_mesh(game->get_meshes()->put("quad", mb2.build_textured_quad()));
		quad_mesh->set_texture(wave_mesh->get_wave_texture());
		quad_mesh->set_program(game->get_shaders()->get_program("sc_ui_texture.vsh", "sc_texture.fsh"));
		quad_mesh->transform.position = { 1.0f, -0.5f, 0.0f };
		quad_mesh->transform.scale = { 0.3f, 0.3f, 1.0f };
		quad_mesh->transform.update();
		quad_mesh->visible = false;		

		auto info_text = game->create_text_mesh(1.0f / 20.0f);
		info_text->transform.position = { -1.5f, 0.0f, 0.0f };
		std::stringstream ss;
		ss << "<#white>" << std::endl 
			<< "<WASD> Move camera" << std::endl
			<< "<Right Mouse Button> Mouselook" << std::endl
			<< "<F1> Toggle Wirframe" << std::endl
			<< "<F2> Toggle Normal Map" << std::endl
			<< "<F3> Re-init with current settings" << std::endl
			<< "<F4> Reset settings" << std::endl
			<< "<F11> Toggle Info" << std::endl
			<< "Modifiers (Shift goes up, non-shift goes down):" << std::endl
			<< "<H> Change Water Height" << std::endl
			<< "<J> Geo Wave Height" << std::endl
			<< "<K> Geo Wave Choppiness" << std::endl			
			<< "<U> Texture Wave Height" << std::endl			
			<< "<Y> Texture Wave Scaling" << std::endl			
			<< "<N> Texture Wave Noise" << std::endl			
			<< "<O> Geo Wave Angle Deviation" << std::endl			
			<< "<P> Texture Wave Angle Deviation" << std::endl			
			<< "<G> Env map radius" << std::endl			
			<< std::endl;
		info_text->set_text(ss.str());
		info_text->transform.update();
		info_mesh = overlay_meshes.add_instance(std::move(info_text));

		game->set_controller(this);
	}

	void WaveScene::init_environment(void)
	{
		env = std::make_unique<Environment>(game, skydome_mesh);

		// Add environment phase to show skydome
        Environment::Phase default_phase;
        default_phase.duration = 6.0f * 60.0f;
        default_phase.mat.ambient = color_rgb(0x0);
        default_phase.mat.diffuse = color_rgb(0x0);
        default_phase.mat.specular = color_rgb(0x0);
        default_phase.mat.custom = Color{5.0f,1.0f,0.0f,0.0f};
        default_phase.color_trans = linear_transition<Color>;
        default_phase.light.position = Vector3{-0.5f, 0.5f, -0.5f};
        default_phase.light_pos_trans = linear_transition<Vector3>;
        default_phase.light.ambient = color_rgb(0x202020);
        default_phase.light.diffuse = color_rgb(0x0);
        default_phase.light.specular = color_rgb(0x0);
        env->add_phase(default_phase);

		env->set_current_phase(0);
	}

	void WaveScene::handle_event(const SDL_Event& e)
	{
		switch (e.type)
		{
		case SDL_MOUSEWHEEL:
		{
			auto camera = game->get_renderer()->get_camera();
			camera->set_distance(camera->get_distance() - 2.0f * (float)e.wheel.y);
			break;
		}
		}
	}

	void WaveScene::handle_keyboard(const SDL_Event & e)
	{
		auto factor = (e.key.keysym.mod & KMOD_SHIFT) ? 0.1f : -0.1f;
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;
		case SDLK_F1:
			game->get_renderer()->toggle_wireframe();
			break;
		case SDLK_F2: // show normal map
			quad_mesh->visible = !quad_mesh->visible;
			break;
		case SDLK_F3: // reinit with current settings
			wave_mesh->init_waves();
			break;
		case SDLK_F4: // reset defaults
			wave_mesh->reset_state();
			wave_mesh->init_waves();
			break;						
		case SDLK_F11:
			info_mesh->visible = !info_mesh->visible;
			break;
		case SDLK_h: // water level
			wave_mesh->set_water_level(wave_mesh->get_water_level() + factor * 3.0f);
			break;
		case SDLK_j: // geometric wave height
			wave_mesh->set_geo_wave_height(wave_mesh->get_geo_wave_height() + factor * 0.05f);
			break;
		case SDLK_k: // geometric wave choppiness
			wave_mesh->set_geo_wave_chop(wave_mesh->get_geo_wave_chop() + factor * 0.05f);
			break;
		case SDLK_u: // texture wave height
			wave_mesh->set_tex_wave_height(wave_mesh->get_tex_wave_height() + factor * 0.05f);
			break;
		case SDLK_y: // texture scaling
			wave_mesh->set_tex_scale(wave_mesh->get_tex_scale() + factor * 1.0f);
			break;
		case SDLK_n: // texture noise
			wave_mesh->set_tex_noise(wave_mesh->get_tex_noise() + factor * 0.1f);
			break;
		case SDLK_o: // geometric wave angle deviation
			wave_mesh->set_geo_angle_dev(wave_mesh->get_geo_angle_dev() + factor * 10.0f);
			break;
		case SDLK_p: // texture wave angle deviation
			wave_mesh->set_tex_angle_dev(wave_mesh->get_tex_angle_dev() + factor * 10.0f);
			break;
		case SDLK_g: // environment map radius
			wave_mesh->set_geo_env_radius(wave_mesh->get_geo_env_radius() * (1.0f + factor * 0.5f));
			break;
		}
	}

	void WaveScene::update(float delta)
	{
		env->update(delta);

		// Camera controls
		auto dev = game->get_devices()->active;
		auto cam = dynamic_cast<OrbitCamera3*>(game->get_renderer()->get_camera());
		camera_target += 10.0f * delta * (dev->ly * cam->transform.dir
				+ dev->lx * cam->transform.right);
		camera_target.y = 0.5f;
		cam->set_look_at(camera_target);

		object_meshes.update(delta);
		overlay_meshes.update(delta);
		wave_mesh->update(delta);
	}

	void WaveScene::render(void)
	{
		std::vector<Mesh*> meshes;
		meshes.push_back(game->get_debug_meshes());
		meshes.push_back(&object_meshes);
		meshes.push_back(wave_mesh.get());
		meshes.push_back(&overlay_meshes);
		game->get_renderer()->render(meshes);
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/waves.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game3 app(settings);
		app.add_scene("main", std::make_unique<dukat::WaveScene>(&app));
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