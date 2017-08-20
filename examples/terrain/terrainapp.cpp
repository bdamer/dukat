// terrain.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "terrainapp.h"

#include <dukat/clipmap.h>
#include <dukat/devicemanager.h>
#include <dukat/diamondsquaregenerator.h>
#include <dukat/firstpersoncamera3.h>
#include <dukat/fixedcamera3.h>
#include <dukat/heightmap.h>
#include <dukat/inputdevice.h>
#include <dukat/log.h>
#include <dukat/mathutil.h>
#include <dukat/meshbuilder2.h>
#include <dukat/meshbuilder3.h>
#include <dukat/orbitcamera3.h>
#include <dukat/ray3.h>
#include <dukat/renderer3.h>
#include <dukat/settings.h>
#include <dukat/sysutil.h>
#include <dukat/vertextypes3.h>

namespace dukat
{
	void Game::init(void)
	{
		MeshBuilder2 builder2;
		MeshBuilder3 builder3;

		Game3::init();

		level_size = settings.get_int("renderer.terrain.size");
		max_levels = settings.get_int("renderer.terrain.levels");

		build_palette();

		renderer->disable_effects();

		// White Directional Light
		auto light0 = renderer->get_light(Renderer3::dir_light_idx);
		light0->position = { 0.0f, -0.5f, 0.5f }; // light direction stored as position
		light0->ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
		light0->diffuse = { 0.4f, 0.4f, 0.4f, 1.0f };
		light0->specular = { 0.4f, 0.4f, 0.4f, 1.0f };

		object_meshes.stage = RenderStage::SCENE;
		object_meshes.visible = true;

		// Origin
		mesh_cache->put("default-axis", builder3.build_axis());
		auto mi = object_meshes.create_instance();
		mi->set_mesh(mesh_cache->get("default-axis"));
		mi->set_program(shader_cache->get_program("sc_color.vsh", "sc_color.fsh"));
		mi->transform.position.y = 0.5f;
		
		// Observer
		mesh_cache->put("observer", builder3.build_sphere(16, 16));
		observer_mesh = object_meshes.create_instance();
		observer_mesh->set_mesh(mesh_cache->get("observer"));
		observer_mesh->set_program(shader_cache->get_program("sc_texture.vsh", "sc_texture.fsh"));
		observer_mesh->set_texture(texture_cache->get("blank.png"));
		Material m;
		m.ambient = { 1.0f, 0.0f, 1.0f, 1.0f };
		observer_mesh->set_material(m);
		// height map always starts at 0 / 0 for lower left corner of inner level
		observer_mesh->transform.position = { 0.5f * (float)(level_size + 2), 10.0f, 0.5f * (float)(level_size + 2) } ;

		auto cursor_quad = mesh_cache->put("cursor", builder2.build_textured_quad({ 0.0f, 0.0f, 1.0f, 1.0f }));
		cursor_mesh = object_meshes.create_instance();
		cursor_mesh->set_mesh(cursor_quad);
		cursor_mesh->set_program(shader_cache->get_program("sc_texture.vsh", "sc_texture.fsh"));
		cursor_mesh->set_texture(texture_cache->get("blank.png"));
		cursor_mesh->transform.rot *= Quaternion().set_to_rotate_x(-pi_over_two);

		generate_terrain();

		// Add skydome mesh as last object mesh
		MeshBuilder3 mb3;
		auto skydome_mesh = object_meshes.create_instance();
		skydome_mesh->set_mesh(mesh_cache->put("skydome", mb3.build_dome(12, 12, true)));
		skydome_mesh->set_program(shader_cache->get_program("sc_skydome.vsh", "sc_skydome.fsh"));
		Material mat;
		mat.ambient = Color{0.66f,0.78f,0.79f,1.0f};
		mat.diffuse = Color{0.22f,0.41f,0.75f,1.0f};
		skydome_mesh->set_material(mat);

		overlay_meshes.stage = RenderStage::OVERLAY;
		overlay_meshes.visible = true;

		auto info_text = create_text_mesh(1.0f / 20.0f);
		info_text->transform.position = { -1.5f, -0.5f, 0.0f };
		std::stringstream ss;
		ss << "<F1> Toggle Wirframe" << std::endl
		   << "<F2> Toggle Culling" << std::endl
		   << "<F3> Toggle Blending" << std::endl
		   << "<F4> Toggle Stitching" << std::endl
		   << "<F5> Toggle Normals" << std::endl
		   << "<F11> Toggle Info" << std::endl
		   << "<1-4> Switch Terain" << std::endl
		   << "<WASD> Move Camera" << std::endl
		   << "<QE> Change altitude" << std::endl
			<< std::endl;
		info_text->set_text(ss.str());
		info_text->transform.update();
		info_mesh = overlay_meshes.add_instance(std::move(info_text));

		debug_meshes.stage = RenderStage::OVERLAY;
		debug_meshes.visible = debug;

		auto debug_text = create_text_mesh(1.0f / 20.0f);
		debug_text->transform.position = { -1.0f, 1.0, 0.0f };
		debug_text->transform.update();
		debug_meshes.add_instance(std::move(debug_text));

		display_level = 0;
		auto quad = mesh_cache->put("quad", builder2.build_textured_quad({ 0.0f, 1.0f, 1.0f, 0.0f}));
		elevation_mesh = debug_meshes.create_instance();
		elevation_mesh->set_mesh(quad);
		elevation_mesh->set_program(shader_cache->get_program("sc_ui_texture.vsh", "sc_clipmap_elevation_texture.fsh"));
		texture = std::make_unique<Texture>();
		texture->id = clip_map->get_elevation_map()->id;
		texture->target = GL_TEXTURE_2D_ARRAY;
		elevation_mesh->set_texture(texture.get());
		debug_mat.custom.r = (float)display_level;
		elevation_mesh->set_material(debug_mat);
		elevation_mesh->transform.position = { 1.0f, -0.5f, 0.0f };
		elevation_mesh->transform.scale = { 0.3f, 0.3f, 1.0f };
		elevation_mesh->transform.update();

		switch_camera_mode(Terrain);
	}

	void Game::build_palette(void)
	{
		const auto palette_size = 256;
		palette.resize(palette_size);

		const auto blue_range = (int)std::floor(palette_size * 0.025f);
		const auto green_range = (int)std::floor(palette_size * 0.075f) + blue_range;
		const auto ocre_range = (int)std::floor(palette_size * 0.10f) + green_range;
		const auto red_range = (int)std::floor(palette_size * 0.80f) + ocre_range;

		int i;
		for (i = 0; i <= blue_range; i++)
		{
			auto perc = (float)i / (float)blue_range;
			palette[i] = Color{ 0.0f, perc, 1.0f - perc, 1.0f };
		}
		for (i = blue_range; i < green_range; i++)
		{
			float perc = (float)(i - blue_range) / (float)(green_range - blue_range);
			palette[i] = Color{ perc * 0.675f, 1.0f - perc * 0.277f, 0.0f, 1.0f };
		}
		for (i = green_range; i < ocre_range; i++)
		{
			float perc = (float)(i - green_range) / (float)(ocre_range - green_range);
			palette[i] = Color{ 0.675f + perc * 0.325f, (1.0f - perc) * 0.723f, 0.0f, 1.0f };
		}
		for (i = ocre_range; i < red_range; i++)
		{
			float perc = (float)(i - ocre_range) / (float)(red_range - ocre_range);
			palette[i] = Color{ 1.0f, perc, perc, 1.0f };
		}
		palette[palette_size - 1] = Color{ 1.0f, 1.0f, 1.0f, 1.0f };
	}

	void Game::load_mtrainier(void)
	{
		// Mt Rainier data set is 10m horizontal resolution, 102.4m vertical for every 0.1f.
		// Note: the data source acknowledges that the data is "squised" when the max range > 1024, so we 
		// stretch it by a factor of 2.
		height_map = std::make_unique<HeightMap>(max_levels, 2.0f * 102.4f);
		height_map->load("../assets/heightmaps/mt_rainier_1k.png");
		clip_map = std::make_unique<ClipMap>(this, max_levels, level_size, height_map.get());
		clip_map->set_program(shader_cache->get_program("sc_clipmap.vsh", "sc_clipmap.fsh"));
		clip_map->set_palette(palette);

		switch_camera_mode(Terrain);
	}

	void Game::load_pugetsound(void)
	{
		// Puget sound data set: 160m horizontal resolution, 0.1m vertical for every 1/65536
		height_map = std::make_unique<HeightMap>(max_levels, 0.1f * 65536.0f / 160.0f);
		height_map->load("../assets/heightmaps/ps_elevation_1k.png");
		//height_map->load("../assets/heightmaps/ps_elevation_4k.png", 0.1f * 65536.0f / 40.0f);
		clip_map = std::make_unique<ClipMap>(this, max_levels, level_size, height_map.get());
		clip_map->set_program(shader_cache->get_program("sc_clipmap.vsh", "sc_clipmap.fsh"));
		clip_map->set_palette(palette);

		switch_camera_mode(Terrain);
	}

	void Game::load_blank(void)
	{
		height_map = std::make_unique<HeightMap>(max_levels, 0.1f * 65536.0f / 160.0f);
		height_map->load("../assets/heightmaps/blank_1k.png");
		clip_map = std::make_unique<ClipMap>(this, max_levels, level_size, height_map.get());
		clip_map->set_program(shader_cache->get_program("sc_clipmap.vsh", "sc_clipmap.fsh"));
		clip_map->set_palette(palette);

		switch_camera_mode(Terrain);
	}

	void Game::generate_terrain(void)
	{
		height_map = std::make_unique<HeightMap>(max_levels, 100.0f);
		DiamondSquareGenerator gen(42);
		gen.set_roughness(250.0f);
		height_map->generate(513, gen);
		clip_map = std::make_unique<ClipMap>(this, max_levels, level_size, height_map.get());
		clip_map->set_program(shader_cache->get_program("sc_clipmap.vsh", "sc_clipmap.fsh"));
		clip_map->set_palette(palette);

		switch_camera_mode(Terrain);
	}

	void Game::switch_camera_mode(CameraMode mode)
	{
		camera_mode = mode;

		if (camera_mode > Overhead)
			camera_mode = Terrain;

		switch (camera_mode)
		{
		case Terrain:
		{
			observer_mesh->visible = true;
			direct_camera_control = false;
			Vector3 target(observer_mesh->transform.position.x, 0.0f, observer_mesh->transform.position.z);
			const Vector3 fixed_camera_offset(0.0f, 100.0f, -100.0f);
			auto camera = std::make_unique<OrbitCamera3>(this, target, 50.0f, 0.0f, pi_over_four);
			camera->set_min_distance(5.0f);
			camera->set_max_distance(100.0f);
			camera->set_vertical_fov(settings.get_float("camera.fov"));
			camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
			camera->refresh();
			renderer->set_camera(std::move(camera));
		}
		break;

		case FirstPerson:
		{
			auto camera = std::make_unique<FirstPersonCamera3>(this);
			camera->transform.position = observer_mesh->transform.position;
			camera->set_vertical_fov(settings.get_float("camera.fov"));
			camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
			camera->refresh();
			camera->set_movement_speed(10.0f);
			renderer->set_camera(std::move(camera));

			observer_mesh->visible = false;
			direct_camera_control = true;
		}
		break;

		case Overhead:
		{
			observer_mesh->visible = true;
			direct_camera_control = false;
			Vector3 target(observer_mesh->transform.position.x, 0.0f, observer_mesh->transform.position.z);
			const Vector3 overhead_camera_offset(0.0f, 200.0f, 0.0f);
			auto camera = std::make_unique<FixedCamera3>(get_window(), target + overhead_camera_offset, target, Vector3::unit_z);
			camera->set_vertical_fov(settings.get_float("camera.fov"));
			camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
			camera->refresh();
			renderer->set_camera(std::move(camera));
		}
		break;	
		}
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
		
		case SDL_MOUSEMOTION:
		{
			auto camera = renderer->get_camera();
			auto ray = camera->pick_ray_screen(e.button.x, e.button.y);
			auto t = height_map->intersect_ray(ray);
			if (t != no_intersection)
			{
				auto intersection = ray.origin + ray.dir * t;
				cursor_mesh->transform.position = intersection;
				// Sample elevation below cursor position
				auto z = height_map->sample(0, cursor_mesh->transform.position.x, cursor_mesh->transform.position.z)
					* height_map->get_scale_factor();
				cursor_mesh->transform.position.y = z + 1.0f;
			}
			break;
		}

		default:
			Game3::handle_event(e);
			break;
		}
	}

	void Game::handle_keyboard(const SDL_Event& e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_F1:
			clip_map->wireframe = !clip_map->wireframe;
			break;		
		case SDLK_F2:
			clip_map->culling = !clip_map->culling;
			break;
		case SDLK_F3:
			clip_map->blending = !clip_map->blending;
			break;
		case SDLK_F4:
			clip_map->stitching = !clip_map->stitching;
			break;
		case SDLK_F5:
			clip_map->lighting = !clip_map->lighting;
			break;
		case SDLK_F11:
			info_mesh->visible = !info_mesh->visible;
			break;

		case SDLK_1:
			load_mtrainier();
			break;
		case SDLK_2:
			load_pugetsound();
			break;
		case SDLK_3:
			load_blank();
			break;
		case SDLK_4:
			generate_terrain();
			break;

		case SDLK_h: // show heightmap texture
			texture->id = clip_map->get_elevation_map()->id;
			elevation_mesh->set_program(shader_cache->get_program("sc_ui_texture.vsh", "sc_clipmap_elevation_texture.fsh"));
			break;
		case SDLK_n: // show normalmap texture 
			texture->id = clip_map->get_normal_map()->id;
			elevation_mesh->set_program(shader_cache->get_program("sc_ui_texture.vsh", "sc_clipmap_normal_texture.fsh"));
			break;
		case SDLK_COMMA:
			display_level = std::max(0, display_level - 1);
			debug_mat.custom.r = (float)display_level;
			elevation_mesh->set_material(debug_mat);
			break;
		case SDLK_PERIOD:
			display_level = std::min(max_levels - 1, display_level + 1);
			debug_mat.custom.r = (float)display_level;
			elevation_mesh->set_material(debug_mat);
			break;

		case SDLK_q:
			{
				auto cam = renderer->get_camera();
				cam->transform.position -= cam->transform.up;	
			}
			break;
		case SDLK_e:
			{
				auto cam = renderer->get_camera();
				cam->transform.position += cam->transform.up;	
			}
			break;

		case SDLK_c:
			switch_camera_mode((CameraMode)(camera_mode + 1));
			break;
		case SDLK_v:
			direct_camera_control = !direct_camera_control;
			break;
			
		default:
			Game3::handle_keyboard(e);
		}
	}

	void Game::update(float delta)
	{
		Game3::update(delta);
		object_meshes.update(delta);
		debug_meshes.update(delta);

		// Sample elevation below observer position
		auto z = height_map->sample(0, observer_mesh->transform.position.x, observer_mesh->transform.position.z) 
			* height_map->get_scale_factor();

		switch (camera_mode)
		{
		case Terrain:
		{
			// Move observer based on camera direction
			auto dev = device_manager->active;
			auto cam = dynamic_cast<OrbitCamera3*>(renderer->get_camera());
			auto offset = 10.0f * delta * (dev->ly * cam->transform.dir
				+ dev->lx * cam->transform.right);
			observer_mesh->transform.position += offset;
			observer_mesh->transform.position.y = z + 1.0f;
			cam->set_look_at(observer_mesh->transform.position);
		}
		break;

		case FirstPerson:
		{
			auto cam = renderer->get_camera();
			if (direct_camera_control)
			{
				//cam->transform.position.y = z + 0.5f;
				observer_mesh->transform.position = cam->transform.position;
				observer_mesh->transform.dir = cam->transform.dir;
			}
		}
		break;

		case Overhead:
		{
			auto dev = device_manager->active;
			auto offset = 10.0f * delta * (dev->ly * observer_mesh->transform.dir 
				- dev->lx * observer_mesh->transform.left);
			observer_mesh->transform.position += offset;
			observer_mesh->transform.position.y = z + 0.5f;
			auto cam = dynamic_cast<FixedCamera3*>(renderer->get_camera());
			cam->set_look_at(observer_mesh->transform.position);
		}
		break;

		}

		clip_map->observer_pos = observer_mesh->transform.position;
		clip_map->update(delta);
	}

	void Game::render(void)
	{
		std::vector<Mesh*> meshes;
		meshes.push_back(&debug_meshes);
		meshes.push_back(&object_meshes);
		meshes.push_back(&overlay_meshes);
		meshes.push_back(clip_map.get());
		renderer->render(meshes);
	}

	void Game::toggle_debug(void)
	{
		Game3::toggle_debug();
		debug_meshes.visible = !debug_meshes.visible;
		if (debug)
		{
			glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
			clip_map->set_program(shader_cache->get_program("sc_clipmap.vsh", "sc_clipmap_debug.fsh"));
		}
		else
		{
			glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
			clip_map->set_program(shader_cache->get_program("sc_clipmap.vsh", "sc_clipmap.fsh"));
		}
	}

	void Game::update_debug_text(void)
	{
		auto cam = renderer->get_camera();
		auto debug_text = dynamic_cast<TextMeshInstance*>(debug_meshes.get_instance(0));
		std::stringstream ss;
		ss << "WIN: " << window->get_width() << "x" << window->get_height()
			<< " FPS: " << get_fps()
			<< " MESH: " << dukat::perfc.avg(dukat::PerformanceCounter::MESHES)
			<< " VERT: " << dukat::perfc.avg(dukat::PerformanceCounter::VERTICES) << std::endl
			<< "FBO: " << dukat::perfc.avg(dukat::PerformanceCounter::FRAME_BUFFERS) 
			<< " CULL: " << clip_map->culling << " BLND: " << clip_map->blending << " LIGH: " << clip_map->lighting
			<< std::endl;
		debug_text->set_text(ss.str());
	}
}

int main(int argc, char** argv)
{
#if OPENGL_VERSION < 30
	dukat::logger << "Unsupported OpenGL version - requires at least 3.0" << std::endl;
	return -2;
#endif
	try
	{
		std::string config = "../assets/terrain.ini";
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