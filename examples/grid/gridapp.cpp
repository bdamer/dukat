// gridapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gridapp.h"

namespace dukat
{
	const int GridScene::grid_size = 64;
	const float GridScene::scale_factor = 25.0f;

	GridScene::GridScene(Game3* game) : game(game)
	{
		auto settings = game->get_settings();
		grid_mesh = std::make_unique<GridMesh>(game, grid_size, scale_factor);
		camera_target.x = camera_target.z = 0.5f * (float)(grid_size * grid_mesh->tile_spacing);

		// White Directional Light
		auto light0 = game->get_renderer()->get_light(Renderer3::dir_light_idx);
		light0->position = { 0.0f, -0.5f, 0.5f }; // light direction stored as position
		light0->ambient = { 0.2f, 0.1f, 0.05f, 1.0f };
		light0->diffuse = { 0.5f, 0.5f, 0.5f, 1.0f };
		light = std::make_unique<OrbitalLight>(600.0f);

		auto camera = std::make_unique<OrbitCamera3>(game, camera_target, 50.0f, 0.0f, pi_over_four);
		camera->set_min_distance(5.0f);
		camera->set_max_distance(100.0f);
		camera->set_vertical_fov(settings.get_float(settings::video_camera_fov));
		camera->set_clip(settings.get_float(settings::video_camera_nearclip), settings.get_float(settings::video_camera_farclip));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));		
		object_meshes.stage = RenderStage::Scene;
		object_meshes.visible = true;

		// Generate heightmap
		heightmap = std::make_unique<HeightMap>(1);
		heightmap->set_scale_factor(scale_factor);
		DiamondSquareGenerator gen(123);
		gen.set_range(0.0f, 1.0f);
		gen.set_roughness(120.0f);
		heightmap->generate(grid_size, gen);
		grid_mesh->load_height_level(heightmap->get_level(0));

		const int texture_size = 1024;
#if OPENGL_VERSION >= 30
		// Generate array containing textures used for splatting
		auto terrain_texture = std::make_unique<Texture>(texture_size, texture_size);
		terrain_texture->target = GL_TEXTURE_2D_ARRAY;
		glBindTexture(GL_TEXTURE_2D_ARRAY, terrain_texture->id);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, texture_size, texture_size, 4,
			0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);

		int i = 0;
		auto sand_surface = game->get_textures()->load_surface("sand01_1024.png");
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i++, texture_size, texture_size, 1,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, sand_surface->get_surface()->pixels);
		auto grass_surface = game->get_textures()->load_surface("grass01_1024.png");
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i++, texture_size, texture_size, 1,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, grass_surface->get_surface()->pixels);
		auto dirt_surface = game->get_textures()->load_surface("dirt01_1024.png");
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i++, texture_size, texture_size, 1,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, dirt_surface->get_surface()->pixels);
		auto rock_surface = game->get_textures()->load_surface("rock01_1024.png");
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i++, texture_size, texture_size, 1,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, rock_surface->get_surface()->pixels);

		glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
#else
		// Generate texture atlas containing textures used for splatting
		auto terrain_texture = std::make_unique<Texture>(2 * texture_size, 2 * texture_size);
		glBindTexture(GL_TEXTURE_2D, terrain_texture->id);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2 * texture_size, 2 * texture_size,
			0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);

		auto sand_surface = game->get_textures()->load_surface("sand01_1024.png");
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_size, texture_size,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, sand_surface->get_surface()->pixels);
		auto grass_surface = game->get_textures()->load_surface("grass01_1024.png");
		glTexSubImage2D(GL_TEXTURE_2D, 0, texture_size, 0, texture_size, texture_size,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, grass_surface->get_surface()->pixels);
		auto dirt_surface = game->get_textures()->load_surface("dirt01_1024.png");
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, texture_size, texture_size, texture_size,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, dirt_surface->get_surface()->pixels);
		auto rock_surface = game->get_textures()->load_surface("rock01_1024.png");
		glTexSubImage2D(GL_TEXTURE_2D, 0, texture_size, texture_size, texture_size, texture_size,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, rock_surface->get_surface()->pixels);
#endif
		grid_mesh->set_terrain_texture(std::move(terrain_texture));

		// Add skydome mesh as last object mesh
		MeshBuilder3 mb3;
		auto skydome_mesh = object_meshes.create_instance();
		skydome_mesh->set_mesh(game->get_meshes()->put("skydome", mb3.build_dome(6, 6, true)));
		skydome_mesh->set_program(game->get_shaders()->get_program("sc_skydome.vsh", "sc_skydome.fsh"));
		Material mat;
		mat.ambient = Color{0.66f,0.78f,0.79f,1.0f};
		mat.diffuse = Color{0.22f,0.41f,0.75f,1.0f};
		skydome_mesh->set_material(mat);

		overlay_meshes.stage = RenderStage::Overlay;
		overlay_meshes.visible = true;

		auto info_text = game->create_text_mesh();
		info_text->set_size(1.0f / 20.0f);
		info_text->transform.position = { -1.5f, -0.5f, 0.0f };
		std::stringstream ss;
		ss << "<,.> Change Grid Spacing" << std::endl
			<< "<F1> Toggle Wirframe" << std::endl
			<< "<F2> Generate Terrain" << std::endl
			<< "<F5> Save Heightmap" << std::endl
			<< "<F6> Load Heightmap" << std::endl
			<< "<F11> Toggle Info" << std::endl
			<< std::endl;
		info_text->set_text(ss.str());
		info_text->update();
		info_mesh = overlay_meshes.add_instance(std::move(info_text));

		game->set_controller(this);
	}

	void GridScene::handle_event(const SDL_Event& e)
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

	void GridScene::handle_keyboard(const SDL_Event & e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;
		case SDLK_F1:
			game->get_renderer()->toggle_wireframe();
			break;
		case SDLK_F2:
			{
			DiamondSquareGenerator gen(std::rand() % 10000);
			gen.set_range(0.0f, 1.0f);
			gen.set_roughness(120.0f);
			heightmap->generate(grid_size, gen);
			grid_mesh->load_height_level(heightmap->get_level(0));
			}
			break;
		case SDLK_F5:
			heightmap->save("heightmap.png");
			break;
		case SDLK_F6:
			heightmap->load("heightmap.png");
			break;
		case SDLK_F11:
			info_mesh->visible = !info_mesh->visible;
			break;
		case SDLK_COMMA:
			grid_mesh->tile_spacing = std::max(1.0f, grid_mesh->tile_spacing - 1.0f);
			break;
		case SDLK_PERIOD:
			grid_mesh->tile_spacing = std::min(32.0f, grid_mesh->tile_spacing + 1.0f);
			break;
		}
	}

	void GridScene::update(float delta)
	{
		auto dev = game->get_devices()->active;
		auto cam = dynamic_cast<OrbitCamera3*>(game->get_renderer()->get_camera());
		camera_target += 10.0f * delta * (dev->ly * cam->transform.dir
				+ dev->lx * cam->transform.right);
		camera_target.y = 0.5f * scale_factor * (float)grid_mesh->tile_spacing;
		cam->set_look_at(camera_target);

		grid_mesh->update(delta);
		object_meshes.update(delta);
		overlay_meshes.update(delta);
	}

	void GridScene::render(void)
	{
		std::vector<Mesh*> meshes;
		meshes.push_back(game->get_debug_meshes());
		meshes.push_back(grid_mesh.get());
		meshes.push_back(&object_meshes);
		meshes.push_back(&overlay_meshes);
		game->get_renderer()->render(meshes);
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/grid.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game3 app(settings);
		app.add_scene("main", std::make_unique<dukat::GridScene>(&app));
		app.push_scene("main");
		return app.run();
	}
	catch (const std::exception& e)
	{
		dukat::log->error("Application failed with error: {}", e.what());
		return -1;
	}
}