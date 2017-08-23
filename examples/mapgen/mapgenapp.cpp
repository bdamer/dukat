// mapgenapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "mapgenapp.h"

#include <dukat/aabb2.h>
#include <dukat/devicemanager.h>
#include <dukat/fixedcamera3.h>
#include <dukat/inputdevice.h>
#include <dukat/log.h>
#include <dukat/mathutil.h>
#include <dukat/meshbuilder3.h>
#include <dukat/quaternion.h>
#include <dukat/renderer3.h>
#include <dukat/settings.h>
#include <dukat/vertextypes3.h>

namespace dukat
{
	std::vector<Vector2> generate_point_set(int count, const AABB2& limits)
	{
		std::vector<Vector2> points(count);
		for (int i = 0; i < count; i++)
		{
			points[i] = Vector2::random(limits.min, limits.max);
		}
		return points;
	}

	void Game::generate_map(void)
	{
		AABB2 limits(Vector2({ -1.0f, -1.0f }), Vector2({ 1.0f, 1.0f }));
		auto points = generate_point_set(polygon_count, limits);
		graph.from_points(points);
		graph.generate();
		graph.create_water_land_mesh(fill_mesh->get_mesh());
		graph.create_edge_mesh(line_mesh->get_mesh());
	}

	void Game::switch_mode(void)
	{
		float z_scale;
		if (render_mode == Overhead)
		{
			auto camera = std::make_unique<FixedCamera3>(this, Vector3{ 0.0f, 2.5f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3::unit_z);
			camera->set_vertical_fov(settings.get_float("camera.fov"));
			camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
			camera->refresh();
			renderer->set_camera(std::move(camera));
			z_scale = 0.0f;
			fill_mesh->transform.rot.identity();
			line_mesh->transform.rot.identity();
		}
		else
		{
			auto camera = std::make_unique<FixedCamera3>(this, Vector3{ 1.5f, 1.5f, 1.5f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3::unit_y);
			camera->set_vertical_fov(settings.get_float("camera.fov"));
			camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
			camera->refresh();
			renderer->set_camera(std::move(camera));
			z_scale = 0.25f;
		}

		switch (map_mode)
		{
		case LandWater:
			graph.create_water_land_mesh(fill_mesh->get_mesh(), z_scale);
			graph.create_edge_mesh(line_mesh->get_mesh(), z_scale);
			break;
		case Elevation:
			graph.create_elevation_mesh(fill_mesh->get_mesh(), z_scale);
			graph.create_edge_mesh(line_mesh->get_mesh(), z_scale);
			break;
		case Moisture:
			graph.create_moisture_mesh(fill_mesh->get_mesh(), z_scale);
			graph.create_river_mesh(line_mesh->get_mesh(), z_scale);
			break;
		case Biomes:
			graph.create_biomes_mesh(fill_mesh->get_mesh(), z_scale);
			graph.create_river_mesh(line_mesh->get_mesh(), z_scale);
			break;
		}
	}

	void Game::init(void)
	{
		Game3::init();

		// Default values
		polygon_count = 100;
		render_mode = Overhead;
		map_mode = LandWater;

		object_meshes.stage = RenderStage::SCENE;
		object_meshes.visible = true;

		// Create meshes to display diagram
		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3, offsetof(VertexPosCol, pos)));
		attr.push_back(VertexAttribute(Renderer::at_color, 4, offsetof(VertexPosCol, col)));

		line_mesh = object_meshes.create_instance();
		line_mesh->set_program(shader_cache->get_program("sc_color.vsh", "sc_color.fsh"));
		line_mesh->set_mesh(mesh_cache->put("lines", std::make_unique<MeshData>(GL_LINES, 2048, 0, attr)));
		line_mesh->transform.position = { 0.0f, 0.01f, 0.0f };

		fill_mesh = object_meshes.create_instance();
		fill_mesh->set_program(shader_cache->get_program("sc_color.vsh", "sc_color.fsh"));
		fill_mesh->set_mesh(mesh_cache->put("triangulation", std::make_unique<MeshData>(GL_TRIANGLES, 2048, 0, attr)));

		generate_map();
		switch_mode();

		overlay_meshes.stage = RenderStage::OVERLAY;
		overlay_meshes.visible = true;

		auto info_text = create_text_mesh(1.0f / 20.0f);
		info_text->transform.position = { -1.5f, -0.5f, 0.0f };
		std::stringstream ss;
		ss << "<#magenta><F1> Show land / water" << std::endl
			<< "<F2> Show elevation" << std::endl
			<< "<F3> Show moisture" << std::endl
			<< "<F4> Show biomes" << std::endl
			<< "<F11> Toggle Info" << std::endl
			<< "<R>ender mode 2D / 3D" << std::endl
			<< "<D>ecrease number of polygons" << std::endl
			<< "<I>ecrease number of polygons" << std::endl
			<< "<G>enerate new map</>" << std::endl
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

	void Game::handle_keyboard(const SDL_Event & e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_g: // generate map
			generate_map();
			switch_mode();
			break;
		case SDLK_r: // render mode 
			render_mode = render_mode == Overhead ? Perspective : Overhead;
			switch_mode();
			break;
		case SDLK_d: // Decrease number of polygons
			polygon_count = std::max(polygon_count - 100, 100);
			generate_map();
			switch_mode();
			break;
		case SDLK_i: // Increase number of polygons
			polygon_count = std::min(polygon_count + 100, 2000);
			generate_map();
			switch_mode();
			break;
		case SDLK_F1: // show land / water
			map_mode = LandWater;
			switch_mode();
			break;
		case SDLK_F2: // show elevation mesh
			map_mode = Elevation;
			switch_mode();
			break;
		case SDLK_F3: // show moisture mesh
			map_mode = Moisture;
			switch_mode();
			break;
		case SDLK_F4: // show biomes mesh
			map_mode = Biomes;
			switch_mode();
			break;
		case SDLK_F11:
			info_mesh->visible = !info_mesh->visible;
			break;
		
		default:
			Game3::handle_keyboard(e);
		}
	}

	void Game::update(float delta)
	{
		Game3::update(delta);

		if (render_mode == Perspective)
		{
			Quaternion q;
			q.set_to_rotate_y(delta);
			fill_mesh->transform.rot *= q;
			line_mesh->transform.rot *= q;
		}

		object_meshes.update(delta);
		overlay_meshes.update(delta);
		debug_meshes.update(delta);
	}

	void Game::render(void)
	{
		std::vector<Mesh*> meshes;
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
		std::string config = "../assets/mapgen.ini";
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