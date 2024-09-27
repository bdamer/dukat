// terrain.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "terrainapp.h"

namespace dukat
{
	TerrainScene::TerrainScene(Game3* game) : game(game)
	{
		MeshBuilder2 builder2;
		MeshBuilder3 builder3;

		auto settings = game->get_settings();
		level_size = settings.get_int("renderer.terrain.size");
		max_levels = settings.get_int("renderer.terrain.levels");

		build_palette();

		// White Directional Light
		auto light0 = game->get_renderer()->get_light(Renderer3::dir_light_idx);
		light0->position = { 0.0f, -0.5f, 0.5f }; // light direction stored as position
		light0->ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
		light0->diffuse = { 0.4f, 0.4f, 0.4f, 1.0f };
		light0->specular = { 0.4f, 0.4f, 0.4f, 1.0f };

		object_meshes.stage = RenderStage::Scene;
		object_meshes.visible = true;

		// Origin
		game->get_meshes()->put("default-axis", builder3.build_axis());
		auto mi = object_meshes.create_instance();
		mi->set_mesh(game->get_meshes()->get("default-axis"));
		mi->set_program(game->get_shaders()->get_program("sc_color.vsh", "sc_color.fsh"));
		mi->transform.position.y = 0.5f;
		
		// Observer
		game->get_meshes()->put("observer", builder3.build_sphere(16, 16));
		observer_mesh = object_meshes.create_instance();
		observer_mesh->set_mesh(game->get_meshes()->get("observer"));
		observer_mesh->set_program(game->get_shaders()->get_program("sc_texture.vsh", "sc_texture.fsh"));
		observer_mesh->set_texture(game->get_textures()->get_or_load("white.png"));
		Material m;
		m.ambient = { 1.0f, 0.0f, 1.0f, 1.0f };
		observer_mesh->set_material(m);
		// height map always starts at 0 / 0 for lower left corner of inner level
		observer_mesh->transform.position = { 0.5f * (float)(level_size + 2), 10.0f, 0.5f * (float)(level_size + 2) } ;

		auto cursor_quad = game->get_meshes()->put("cursor", builder2.build_textured_quad({ 0.0f, 0.0f, 1.0f, 1.0f }));
		cursor_mesh = object_meshes.create_instance();
		cursor_mesh->set_mesh(cursor_quad);
		cursor_mesh->set_program(game->get_shaders()->get_program("sc_texture.vsh", "sc_texture.fsh"));
		cursor_mesh->set_texture(game->get_textures()->get_or_load("white.png"));
		cursor_mesh->transform.rot *= Quaternion().set_to_rotate_x(-pi_over_two);

		generate_terrain();

		// Add skydome mesh as last object mesh
		MeshBuilder3 mb3;
		auto skydome_mesh = object_meshes.create_instance();
		skydome_mesh->set_mesh(game->get_meshes()->put("skydome", mb3.build_dome(12, 12, true)));
		skydome_mesh->set_program(game->get_shaders()->get_program("sc_skydome.vsh", "sc_skydome.fsh"));
		Material mat;
		mat.ambient = Color{0.66f,0.78f,0.79f,1.0f};
		mat.diffuse = Color{0.22f,0.41f,0.75f,1.0f};
		skydome_mesh->set_material(mat);

		overlay_meshes.stage = RenderStage::Overlay;
		overlay_meshes.visible = true;

		auto info_text = game->create_text_mesh();
		info_text->set_size(1.0f / 20.0f);
		info_text->transform.position = { -1.6f, -0.3f, 0.0f };

		const auto info =
			"<1-4> Switch Terain\n"
			"<C> Switch Camera View\n"
			"<WASD> Move Camera\n"
			"<QE> Change Altitude\n"
			"<F2> Toggle Wirframe\n"
			"<F3> Toggle Culling\n"
			"<F4> Toggle Blending\n"
			"<F5> Toggle Stitching\n"
			"<F6> Toggle Normals\n"
			"<F11> Toggle Info";
		info_text->set_text(info);
		info_text->transform.update();
		info_mesh = overlay_meshes.add_instance(std::move(info_text));

		auto options_text = game->create_text_mesh();
		options_text->set_size(1.0f / 20.0f);
		options_text->transform.position = { -1.6f, -0.2f, 0.0f };
		auto options_mesh = static_cast<TextMeshInstance*>(game->get_debug_meshes()->add_instance(std::move(options_text)));
		game->get<TimerManager>()->create(1.0f, [this,options_mesh]() {
			std::stringstream ss;
			ss << " CULL: " << clip_map->culling << " BLND: " << clip_map->blending << " LIGH: " << clip_map->lighting << std::endl;
			options_mesh->set_text(ss.str());
		}, true);

		display_level = 0;
		auto quad = game->get_meshes()->put("quad", builder2.build_textured_quad({ 0.0f, 1.0f, 1.0f, 0.0f}));
		elevation_mesh = game->get_debug_meshes()->create_instance();
		elevation_mesh->set_mesh(quad);
		elevation_mesh->set_program(game->get_shaders()->get_program("sc_ui_texture.vsh", "sc_clipmap_elevation_texture.fsh"));
		texture = std::make_unique<Texture>(clip_map->get_elevation_map()->id);
		texture->target = GL_TEXTURE_2D_ARRAY;
		elevation_mesh->set_texture(texture.get());
		debug_mat.custom.r = (float)display_level;
		elevation_mesh->set_material(debug_mat);
		elevation_mesh->transform.position = { 1.0f, -0.5f, 0.0f };
		elevation_mesh->transform.scale = { 0.3f, 0.3f, 1.0f };
		elevation_mesh->transform.update();

		switch_camera_mode(Terrain);

		game->set_controller(this);
	}

	void TerrainScene::build_palette(void)
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

	void TerrainScene::load_mtrainier(void)
	{
		// Mt Rainier data set is 10m horizontal resolution, 102.4m vertical for every 0.1f.
		// Note: the data source acknowledges that the data is "squised" when the max range > 1024, so we 
		// stretch it by a factor of 2.
		height_map = std::make_unique<HeightMap>(max_levels, 2.0f * 102.4f);
		height_map->load("../assets/heightmaps/mt_rainier_1k.png");
		clip_map = std::make_unique<ClipMap>(game, max_levels, level_size, height_map.get());
		clip_map->set_program(game->get_shaders()->get_program("sc_clipmap.vsh", "sc_clipmap.fsh"));
		clip_map->set_palette(palette);

		switch_camera_mode(Terrain);
	}

	void TerrainScene::load_pugetsound(void)
	{
		// Puget sound data set: 160m horizontal resolution, 0.1m vertical for every 1/65536
		height_map = std::make_unique<HeightMap>(max_levels, 0.1f * 65536.0f / 160.0f);
		height_map->load("../assets/heightmaps/ps_elevation_1k.png");
		//height_map->load("../assets/heightmaps/ps_elevation_4k.png", 0.1f * 65536.0f / 40.0f);
		clip_map = std::make_unique<ClipMap>(game, max_levels, level_size, height_map.get());
		clip_map->set_program(game->get_shaders()->get_program("sc_clipmap.vsh", "sc_clipmap.fsh"));
		clip_map->set_palette(palette);

		switch_camera_mode(Terrain);
	}

	void TerrainScene::load_blank(void)
	{
		height_map = std::make_unique<HeightMap>(max_levels, 0.1f * 65536.0f / 160.0f);
		height_map->load("../assets/heightmaps/blank_1k.png");
		clip_map = std::make_unique<ClipMap>(game, max_levels, level_size, height_map.get());
		clip_map->set_program(game->get_shaders()->get_program("sc_clipmap.vsh", "sc_clipmap.fsh"));
		clip_map->set_palette(palette);

		switch_camera_mode(Terrain);
	}

	void TerrainScene::generate_terrain(void)
	{
		height_map = std::make_unique<HeightMap>(max_levels, 100.0f);
		DiamondSquareGenerator gen(42);
		gen.set_roughness(250.0f);
		height_map->generate(513, gen);
		clip_map = std::make_unique<ClipMap>(game, max_levels, level_size, height_map.get());
		clip_map->set_program(game->get_shaders()->get_program("sc_clipmap.vsh", "sc_clipmap.fsh"));
		clip_map->set_palette(palette);

		switch_camera_mode(Terrain);
	}

	void TerrainScene::switch_camera_mode(CameraMode mode)
	{
		camera_mode = mode;

		if (camera_mode > Overhead)
			camera_mode = Terrain;

		auto settings = game->get_settings();
		switch (camera_mode)
		{
		case Terrain:
		{
			observer_mesh->visible = true;
			direct_camera_control = false;
			Vector3 target(observer_mesh->transform.position.x, 0.0f, observer_mesh->transform.position.z);
			const Vector3 fixed_camera_offset(0.0f, 100.0f, -100.0f);
			auto camera = std::make_unique<OrbitCamera3>(game, target, 50.0f, 0.0f, pi_over_four);
			camera->set_min_distance(5.0f);
			camera->set_max_distance(100.0f);
			camera->set_vertical_fov(settings.get_float("camera.fov"));
			camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
			camera->refresh();
			game->get_renderer()->set_camera(std::move(camera));
		}
		break;

		case FirstPerson:
		{
			auto camera = std::make_unique<FirstPersonCamera3>(game);
			camera->transform.position = observer_mesh->transform.position;
			camera->set_vertical_fov(settings.get_float("camera.fov"));
			camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
			camera->refresh();
			camera->set_movement_speed(10.0f);
			game->get_renderer()->set_camera(std::move(camera));

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
			auto camera = std::make_unique<FixedCamera3>(game, target + overhead_camera_offset, target, Vector3::unit_z);
			camera->set_vertical_fov(settings.get_float("camera.fov"));
			camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
			camera->refresh();
			game->get_renderer()->set_camera(std::move(camera));
		}
		break;	
		}
	}

	void TerrainScene::handle_event(const SDL_Event& e)
	{
		switch (e.type)
		{
		case SDL_MOUSEWHEEL:
		{
			auto camera = game->get_renderer()->get_camera();
			camera->set_distance(camera->get_distance() - 2.0f * (float)e.wheel.y);
			break;
		}
		case SDL_MOUSEMOTION:
		{
			auto camera = game->get_renderer()->get_camera();
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
		}
	}

	void TerrainScene::handle_keyboard(const SDL_Event& e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;
		case SDLK_F2:
			game->get_renderer()->toggle_wireframe();
			break;		
		case SDLK_F3:
			clip_map->culling = !clip_map->culling;
			break;
		case SDLK_F4:
			clip_map->blending = !clip_map->blending;
			break;
		case SDLK_F5:
			clip_map->stitching = !clip_map->stitching;
			break;
		case SDLK_F6:
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
			elevation_mesh->set_program(game->get_shaders()->get_program("sc_ui_texture.vsh", "sc_clipmap_elevation_texture.fsh"));
			break;
		case SDLK_n: // show normalmap texture 
			texture->id = clip_map->get_normal_map()->id;
			elevation_mesh->set_program(game->get_shaders()->get_program("sc_ui_texture.vsh", "sc_clipmap_normal_texture.fsh"));
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
				auto cam = game->get_renderer()->get_camera();
				cam->transform.position -= cam->transform.up;	
			}
			break;
		case SDLK_e:
			{
				auto cam = game->get_renderer()->get_camera();
				cam->transform.position += cam->transform.up;	
			}
			break;

		case SDLK_c:
			switch_camera_mode((CameraMode)(camera_mode + 1));
			break;
		case SDLK_v:
			direct_camera_control = !direct_camera_control;
			break;
		}
	}

	void TerrainScene::update(float delta)
	{
		object_meshes.update(delta);
		overlay_meshes.update(delta);

		// Sample elevation below observer position
		auto z = height_map->sample(0, observer_mesh->transform.position.x, observer_mesh->transform.position.z) 
			* height_map->get_scale_factor();

		switch (camera_mode)
		{
		case Terrain:
		{
			// Move observer based on camera direction
			auto dev = game->get_devices()->active;
			auto cam = dynamic_cast<OrbitCamera3*>(game->get_renderer()->get_camera());
			auto offset = 10.0f * delta * (dev->ly * cam->transform.dir
				+ dev->lx * cam->transform.right);
			observer_mesh->transform.position += offset;
			observer_mesh->transform.position.y = z + 1.0f;
			cam->set_look_at(observer_mesh->transform.position);
			cam->set_rot_speed(4.0f);
		}
		break;

		case FirstPerson:
		{
			auto cam = game->get_renderer()->get_camera();
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
			auto dev = game->get_devices()->active;
			auto offset = 10.0f * delta * (dev->ly * observer_mesh->transform.dir 
				- dev->lx * observer_mesh->transform.left);
			observer_mesh->transform.position += offset;
			observer_mesh->transform.position.y = z + 0.5f;
			auto cam = dynamic_cast<FixedCamera3*>(game->get_renderer()->get_camera());
			cam->set_look_at(observer_mesh->transform.position);
		}
		break;

		}

		clip_map->observer_pos = observer_mesh->transform.position;
		clip_map->update(delta);
	}

	void TerrainScene::render(void)
	{
		std::vector<Mesh*> meshes;
		meshes.push_back(game->get_debug_meshes());
		meshes.push_back(&object_meshes);
		meshes.push_back(&overlay_meshes);
		meshes.push_back(clip_map.get());
		game->get_renderer()->render(meshes);
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
		dukat::Game3 app(settings);
		app.add_scene("main", std::make_unique<dukat::TerrainScene>(&app));
		app.push_scene("main");
		return app.run();
	}
	catch (const std::exception& e)
	{
		dukat::log->error("Application failed with error: {}", e.what());
		return -1;
	}
}