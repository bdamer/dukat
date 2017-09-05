// wavesapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "wavesapp.h"

namespace dukat
{
	constexpr int Game::grid_size;

	void Game::init(void)
	{
		MeshBuilder3 mb3;

		Game3::init();

		renderer->disable_effects();

		glClearColor(0.0f, 0.1f, 0.5f, 1.0f );

		// Create scene objects
		auto camera = std::make_unique<OrbitCamera3>(this, camera_target, 50.0f, 0.0f, pi_over_four);
		camera->set_min_distance(5.0f);
		camera->set_max_distance(100.0f);
		camera->set_vertical_fov(settings.get_float("camera.fov"));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		renderer->set_camera(std::move(camera));		

		object_meshes.stage = RenderStage::SCENE;
		object_meshes.visible = true;

		// load environment map
		auto env_map = texture_cache->get("sorsea.dds");

		// Wave mesh
		wave_mesh = std::make_unique<WaveMesh>(this, grid_size);
		wave_mesh->tile_spacing = 4.0f;
		wave_mesh->set_env_map(env_map);

		// Skydome
		skydome_mesh = object_meshes.create_instance();
		skydome_mesh->set_mesh(mesh_cache->put("skydome", mb3.build_dome(32, 24, true)));
		skydome_mesh->set_program(shader_cache->get_program("sc_skydome.vsh", "sc_skydome.fsh"));
		skydome_mesh->set_texture(env_map, 0);

		init_environment();
		
		overlay_meshes.stage = RenderStage::OVERLAY;
		overlay_meshes.visible = true;

		// Quad used to visualize normal map
		MeshBuilder2 mb2;
		quad_mesh = overlay_meshes.create_instance();
		quad_mesh->set_mesh(mesh_cache->put("quad", mb2.build_textured_quad()));
		quad_mesh->set_texture(wave_mesh->get_wave_texture());
		quad_mesh->set_program(shader_cache->get_program("sc_ui_texture.vsh", "sc_texture.fsh"));
		quad_mesh->transform.position = { 1.0f, -0.5f, 0.0f };
		quad_mesh->transform.scale = { 0.3f, 0.3f, 1.0f };
		quad_mesh->transform.update();
		quad_mesh->visible = false;		

		auto info_text = create_text_mesh(1.0f / 20.0f);
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

		debug_meshes.stage = RenderStage::OVERLAY;
		debug_meshes.visible = debug;

		auto debug_text = create_text_mesh(1.0f / 20.0f);
		debug_text->transform.position.x = -1.0f;
		debug_text->transform.position.y = 1.0f;
		debug_text->transform.update();
		debug_meshes.add_instance(std::move(debug_text));
	}

	void Game::init_environment(void)
	{
		env = std::make_unique<Environment>(this, skydome_mesh);

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

	void Game::handle_event(const SDL_Event& e)
	{
		switch (e.type)
		{
		case SDL_MOUSEWHEEL:
		{
			auto camera = renderer->get_camera();
			camera->set_distance(camera->get_distance() - 2.0f * (float)e.wheel.y);
			break;
		}
		default:
			Game3::handle_event(e);
			break;		
		}
	}

	void Game::handle_keyboard(const SDL_Event & e)
	{
		auto factor = (e.key.keysym.mod & KMOD_SHIFT) ? 0.1f : -0.1f;
		switch (e.key.keysym.sym)
		{
		case SDLK_F1:
			renderer->toggle_wireframe();
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
		default:
			Game3::handle_keyboard(e);
		}
	}

	void Game::update(float delta)
	{
		Game3::update(delta);
		env->update(delta);

		// Camera controls
		auto dev = device_manager->active;
		auto cam = dynamic_cast<OrbitCamera3*>(renderer->get_camera());
		camera_target += 10.0f * delta * (dev->ly * cam->transform.dir
				+ dev->lx * cam->transform.right);
		camera_target.y = 0.5f;
		cam->set_look_at(camera_target);

		object_meshes.update(delta);
		overlay_meshes.update(delta);
		debug_meshes.update(delta);
		wave_mesh->update(delta);
	}

	void Game::render(void)
	{
		std::vector<Mesh*> meshes;
		meshes.push_back(&debug_meshes);
		meshes.push_back(&object_meshes);
		meshes.push_back(wave_mesh.get());
		meshes.push_back(&overlay_meshes);
		renderer->render(meshes);
	}

	void Game::toggle_debug(void)
	{
		Game3::toggle_debug();
		debug_meshes.visible = !debug_meshes.visible;
	}

	void Game::update_debug_text(void)
	{
		std::stringstream ss;
		auto cam = renderer->get_camera();
		ss << "WIN: " << window->get_width() << "x" << window->get_height()
			<< " FPS: " << get_fps()
			<< " MESH: " << dukat::perfc.avg(dukat::PerformanceCounter::MESHES)
			<< " VERT: " << dukat::perfc.avg(dukat::PerformanceCounter::VERTICES) << std::endl;
		auto debug_text = dynamic_cast<TextMeshInstance*>(debug_meshes.get_instance(0));
		debug_text->set_text(ss.str());
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