// gridapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gridapp.h"

#include <dukat/blockbuilder.h>
#include <dukat/devicemanager.h>
#include <dukat/diamondsquaregenerator.h>
#include <dukat/fixedcamera3.h>
#include <dukat/inputdevice.h>
#include <dukat/log.h>
#include <dukat/mathutil.h>
#include <dukat/meshbuilder2.h>
#include <dukat/meshbuilder3.h>
#include <dukat/orbitcamera3.h>
#include <dukat/renderer3.h>
#include <dukat/settings.h>
#include <dukat/vertextypes3.h>

namespace dukat
{
	const int Game::grid_size = 64;
	const float Game::scale_factor = 25.0f;

	void Game::init(void)
	{
		Game3::init();

		renderer->disable_effects();
	
		camera_target.x = camera_target.z = 0.5f * (float)(grid_size * tile_spacing);

		auto camera = std::make_unique<OrbitCamera3>(this, camera_target, 50.0f, 0.0f, pi_over_four);
		camera->set_min_distance(5.0f);
		camera->set_max_distance(100.0f);
		camera->set_vertical_fov(settings.get_float("camera.fov"));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		renderer->set_camera(std::move(camera));		

		object_meshes.stage = RenderStage::SCENE;
		object_meshes.mat_model.identity();
		object_meshes.visible = true;

		// Generate heightmap
		heightmap = std::make_unique<HeightMap>(1);
		heightmap->set_scale_factor(scale_factor);
		DiamondSquareGenerator gen(123);
		gen.set_range(0.0f, 1.0f);
		gen.set_roughness(120.0f);
		heightmap->generate(grid_size, gen);

		// Create elevation texture
		heightmap_texture = std::make_unique<Texture>(grid_size, grid_size, ProfileLinear);
		glBindTexture(GL_TEXTURE_2D, heightmap_texture->id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		auto& level = heightmap->get_level(0);
#if OPENGL_VERSION >= 30
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, grid_size, grid_size, 0, GL_RED, GL_FLOAT, level.data.data());
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, grid_size, grid_size, 0, GL_RED, GL_FLOAT, level.data.data());
#endif

		// Generate mesh for terrain grid
		BlockBuilder bb;
		bb.add_block(grid_size, grid_size);
		mesh_cache->put("grid", bb.create_mesh());
		grid_mesh = object_meshes.create_instance();
		grid_mesh->set_mesh(mesh_cache->get("grid"));
		grid_mesh->set_program(shader_cache->get_program("sc_heightmap.vsh", "sc_heightmap.fsh"));
		grid_mesh->set_texture(heightmap_texture.get(), 0);

		const int texture_size = 1024;

#if OPENGL_VERSION >= 30
		// Generate array containing textures used for splatting
		terrain_texture = std::make_unique<Texture>(texture_size, texture_size);
		terrain_texture->target = GL_TEXTURE_2D_ARRAY;
		glBindTexture(GL_TEXTURE_2D_ARRAY, terrain_texture->id);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, texture_size, texture_size, 4,
			0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);

		int i = 0;
		auto sand_surface = texture_cache->load_surface("sand01_1024.png");
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i++, texture_size, texture_size, 1,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, sand_surface->get_surface()->pixels);
		auto grass_surface = texture_cache->load_surface("grass01_1024.png");
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i++, texture_size, texture_size, 1,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, grass_surface->get_surface()->pixels);
		auto dirt_surface = texture_cache->load_surface("dirt01_1024.png");
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i++, texture_size, texture_size, 1,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, dirt_surface->get_surface()->pixels);
		auto rock_surface = texture_cache->load_surface("rock01_1024.png");
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i++, texture_size, texture_size, 1,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, rock_surface->get_surface()->pixels);

		glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
#else
		// Generate texture atlas containing textures used for splatting
		terrain_texture = std::make_unique<Texture>(2 * texture_size, 2 * texture_size);
		glBindTexture(GL_TEXTURE_2D, terrain_texture->id);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2 * texture_size, 2 * texture_size,
			0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);

		auto sand_surface = texture_cache->load_surface("sand01_1024.png");
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_size, texture_size,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, sand_surface->get_surface()->pixels);
		auto grass_surface = texture_cache->load_surface("grass01_1024.png");
		glTexSubImage2D(GL_TEXTURE_2D, 0, texture_size, 0, texture_size, texture_size,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, grass_surface->get_surface()->pixels);
		auto dirt_surface = texture_cache->load_surface("dirt01_1024.png");
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, texture_size, texture_size, texture_size,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, dirt_surface->get_surface()->pixels);
		auto rock_surface = texture_cache->load_surface("rock01_1024.png");
		glTexSubImage2D(GL_TEXTURE_2D, 0, texture_size, texture_size, texture_size, texture_size,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, rock_surface->get_surface()->pixels);
#endif

		grid_mesh->set_texture(terrain_texture.get(), 1);

		// Add skydome mesh as last object mesh
		MeshBuilder3 mb3;
		auto skydome_mesh = object_meshes.create_instance();
		skydome_mesh->set_mesh(mesh_cache->put("skydome", mb3.build_dome(6, 6, true)));
		skydome_mesh->set_program(shader_cache->get_program("sc_skydome.vsh", "sc_skydome.fsh"));
		Material mat;
		mat.ambient = Color{0.66f,0.78f,0.79f,1.0f};
		mat.diffuse = Color{0.22f,0.41f,0.75f,1.0f};
		skydome_mesh->set_material(mat);

		overlay_meshes.stage = RenderStage::OVERLAY;
		overlay_meshes.visible = true;
		overlay_meshes.mat_model.identity();

		auto info_text = create_text_mesh(1.0f / 20.0f);
		info_text->transform.position = { -1.5f, -0.5f, 0.0f };
		std::stringstream ss;
		ss << "<#white>"
			<< "<,.> Change Grid Spacing" << std::endl
			<< "<F1> Toggle Wirframe" << std::endl
			<< "<F5> Save Heightmap" << std::endl
			<< "<F6> Load Heightmap" << std::endl
			<< "<F11> Toggle Info" << std::endl
			<< std::endl;
		info_text->set_text(ss.str());
		info_text->transform.update();
		info_mesh = overlay_meshes.add_instance(std::move(info_text));
		
		debug_meshes.stage = RenderStage::OVERLAY;
		debug_meshes.visible = debug;
		debug_meshes.mat_model.identity();

		auto debug_text = create_text_mesh(1.0f / 20.0f);
		debug_text->transform.position.x = -1.0f;
		debug_text->transform.position.y = 1.0f;
		debug_text->transform.update();
		debug_meshes.add_instance(std::move(debug_text));
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
		switch (e.key.keysym.sym)
		{
		case SDLK_F1:
			renderer->toggle_wireframe();
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
			tile_spacing = std::max(1, tile_spacing - 1);
			break;
		case SDLK_PERIOD:
			tile_spacing = std::min(32, tile_spacing + 1);
			break;

		default:
			Game3::handle_keyboard(e);
		}
	}

	void Game::update(float delta)
	{
		Game3::update(delta);

		auto dev = device_manager->active;
		auto cam = dynamic_cast<OrbitCamera3*>(renderer->get_camera());
		camera_target += 10.0f * delta * (dev->ly * cam->transform.dir
				+ dev->lx * cam->transform.right);
		camera_target.y = 0.5f * scale_factor * (float)tile_spacing;
		cam->set_look_at(camera_target);

		object_meshes.update(delta);
		overlay_meshes.update(delta);
		debug_meshes.update(delta);
		
		// We're repurposing the model matrix to pass in information about the location and scale of the grid.
		Matrix4 model;
		model.identity();
		// grid scale at current level
		model.m[0] = model.m[1] = (float)tile_spacing;
		// origin of current block in world-space
		model.m[2] = model.m[3] = 0.0f;
		// 1 / texture width,height
		model.m[4] = model.m[5] = 1.0f / (float)grid_size;
		// ZScale of height map 
		model.m[13] = heightmap->get_scale_factor() * (float)tile_spacing;
		// override modelview matrix for grid with custom values
		grid_mesh->transform.mat_model = model;
	}

	void Game::render(void)
	{
		std::vector<Renderable*> meshes;
		meshes.push_back(&debug_meshes);
		meshes.push_back(&object_meshes);
		meshes.push_back(&overlay_meshes);
		renderer->render(meshes);
	}

	void Game::toggle_debug(void)
	{
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
		std::string config = "../assets/grid.ini";
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