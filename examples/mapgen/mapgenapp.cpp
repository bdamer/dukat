// mapgenapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "mapgenapp.h"

namespace dukat
{
    // Define mesh colors
    const Color land_color = { 0.59f, 0.53f, 0.44f, 1.0f };
    const Color ocean_color = { 0.25f, 0.25f, 0.46f, 1.0f };
    const Color lake_color = { 0.12f, 0.56f, 1.0f, 1.0f };
    const Color ice_color{0.97f, 0.97f, 0.97f, 1.0f};
    const Color tropical_rf_color{0.18f, 0.44f, 0.31f, 1.0f};
    const Color temperate_df_color{0.53f, 0.67f, 0.34f, 1.0f};
    const Color grassland_color{0.75f, 0.79f, 0.58f, 1.0f};
    const Color bare_color{0.73f, 0.73f, 0.73f, 1.0f};
    const Color scorched_color{0.6f, 0.6f, 0.6f, 1.0f};
    const Color tundra_color{0.87f, 0.87f, 0.73f, 1.0f};
    const Color marsh_color{0.27f, 0.39f, 0.32f, 1.0f};

	std::vector<Vector2> generate_point_set(int count, const AABB2& limits)
	{
		std::vector<Vector2> points(count);
		for (int i = 0; i < count; i++)
		{
			points[i] = Vector2::random(limits.min, limits.max);
		}
		return points;
	}

	MapgenScene::MapgenScene(Game3* game) : game(game)
	{
		// Default values
		polygon_count = 500;
		render_mode = Overhead;
		map_mode = LandWater;

		object_meshes.stage = RenderStage::SCENE;
		object_meshes.visible = true;

		// Create meshes to display diagram
		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3, offsetof(Vertex3PC, px)));
		attr.push_back(VertexAttribute(Renderer::at_color, 4, offsetof(Vertex3PC, cr)));

		line_mesh = object_meshes.create_instance();
		line_mesh->set_program(game->get_shaders()->get_program("sc_color.vsh", "sc_color.fsh"));
		line_mesh->set_mesh(game->get_meshes()->put("lines", std::make_unique<MeshData>(GL_LINES, 2048, 0, attr)));
		line_mesh->transform.position = { 0.0f, 0.01f, 0.0f };

		fill_mesh = object_meshes.create_instance();
		fill_mesh->set_program(game->get_shaders()->get_program("sc_color.vsh", "sc_color.fsh"));
		fill_mesh->set_mesh(game->get_meshes()->put("triangulation", std::make_unique<MeshData>(GL_TRIANGLES, 2048, 0, attr)));

		generate_map();
		switch_mode();

		overlay_meshes.stage = RenderStage::OVERLAY;
		overlay_meshes.visible = true;

		auto info_text = game->create_text_mesh();
		info_text->set_size(1.0f / 20.0f);
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

		game->set_controller(this);
	}

	void MapgenScene::generate_map(void)
	{
		AABB2 limits(Vector2({ -1.0f, -1.0f }), Vector2({ 1.0f, 1.0f }));
		auto points = generate_point_set(polygon_count, limits);
		graph.from_points(points);
		graph.generate();
		create_water_land_mesh(fill_mesh->get_mesh());
		create_edge_mesh(line_mesh->get_mesh());
	}

	void MapgenScene::switch_mode(void)
	{
		float z_scale;
		auto settings = game->get_settings();
		if (render_mode == Overhead)
		{
			auto camera = std::make_unique<FixedCamera3>(game, Vector3{ 0.0f, 2.5f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3::unit_z);
			camera->set_vertical_fov(settings.get_float("camera.fov"));
			camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
			camera->refresh();
			game->get_renderer()->set_camera(std::move(camera));
			z_scale = 0.0f;
			fill_mesh->transform.rot.identity();
			line_mesh->transform.rot.identity();
		}
		else
		{
			auto camera = std::make_unique<FixedCamera3>(game, Vector3{ 1.5f, 1.5f, 1.5f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3::unit_y);
			camera->set_vertical_fov(settings.get_float("camera.fov"));
			camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
			camera->refresh();
			game->get_renderer()->set_camera(std::move(camera));
			z_scale = 0.25f;
		}

		switch (map_mode)
		{
		case LandWater:
			create_water_land_mesh(fill_mesh->get_mesh(), z_scale);
			create_edge_mesh(line_mesh->get_mesh(), z_scale);
			break;
		case Elevation:
			create_elevation_mesh(fill_mesh->get_mesh(), z_scale);
			create_edge_mesh(line_mesh->get_mesh(), z_scale);
			break;
		case Moisture:
			create_moisture_mesh(fill_mesh->get_mesh(), z_scale);
			create_river_mesh(line_mesh->get_mesh(), z_scale);
			break;
		case Biomes:
			create_biomes_mesh(fill_mesh->get_mesh(), z_scale);
			create_river_mesh(line_mesh->get_mesh(), z_scale);
			break;
		}
	}

	void MapgenScene::handle_keyboard(const SDL_Event & e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;
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
		}
	}

	void MapgenScene::update(float delta)
	{
		if (render_mode == Perspective)
		{
			Quaternion q;
			q.set_to_rotate_y(delta);
			fill_mesh->transform.rot *= q;
			line_mesh->transform.rot *= q;
		}

		object_meshes.update(delta);
		overlay_meshes.update(delta);
	}

	void MapgenScene::create_water_land_mesh(MeshData* mesh, float z_scale)
    {
		std::vector<Vertex3PC> verts;
        Color c;
        for (const auto& p : graph.get_centers())
        {
            if (p->ocean)
            {
                c = ocean_color;
            }
            else if (p->water)
            {
                c = lake_color;
            }
            else
            {
                c = land_color;
            }

            for (auto i = 1u; i < p->corners.size(); i++)
            {
				verts.push_back({ p->pos.x, p->elevation * z_scale, p->pos.y, c.r, c.g, c.b, c.a });
				verts.push_back({ p->corners[i]->pos.x, p->corners[i]->elevation * z_scale, p->corners[i]->pos.y, c.r, c.g, c.b, c.a });
				verts.push_back({ p->corners[i - 1]->pos.x, p->corners[i - 1]->elevation * z_scale, p->corners[i - 1]->pos.y, c.r, c.g, c.b, c.a });
            }
            
			verts.push_back({ p->pos.x, p->elevation * z_scale, p->pos.y, c.r, c.g, c.b, c.a });
			verts.push_back({ p->corners[0]->pos.x, p->corners[0]->elevation * z_scale, p->corners[0]->pos.y, c.r, c.g, c.b, c.a });
			verts.push_back({ p->corners[p->corners.size() - 1]->pos.x, p->corners[p->corners.size() - 1]->elevation * z_scale,
				p->corners[p->corners.size() - 1]->pos.y, c.r, c.g, c.b, c.a });
        }

		mesh->set_vertices(reinterpret_cast<GLfloat*>(verts.data()), verts.size());
    }

    void MapgenScene::create_elevation_mesh(MeshData* mesh, float z_scale)
    {
        std::vector<Vertex3PC> verts;
        Color c{0.0f, 0.0f, 0.0f, 1.0f};
        for (const auto& p : graph.get_centers())
        {
            c.r = p->elevation;
            c.g = p->elevation == 0.0f ? 0.0f : 1.0f;
            c.b = p->elevation == 0.0f ? 1.0f : p->elevation;
            for (auto i = 1u; i < p->corners.size(); i++)
            {
				verts.push_back({ p->pos.x, p->elevation * z_scale, p->pos.y, c.r, c.g, c.b, c.a });
				verts.push_back({ p->corners[i]->pos.x, p->corners[i]->elevation * z_scale, p->corners[i]->pos.y, c.r, c.g, c.b, c.a });
				verts.push_back({ p->corners[i - 1]->pos.x, p->corners[i - 1]->elevation * z_scale, p->corners[i - 1]->pos.y, c.r, c.g, c.b, c.a });
            }
            
			verts.push_back({ p->pos.x, p->elevation * z_scale, p->pos.y, c.r, c.g, c.b, c.a });
			verts.push_back({ p->corners[0]->pos.x, p->corners[0]->elevation * z_scale, p->corners[0]->pos.y, c.r, c.g, c.b, c.a });
			verts.push_back({ p->corners[p->corners.size() - 1]->pos.x, p->corners[p->corners.size() - 1]->elevation * z_scale,
				p->corners[p->corners.size() - 1]->pos.y, c.r, c.g, c.b, c.a });
        }

		mesh->set_vertices(reinterpret_cast<GLfloat*>(verts.data()), verts.size());
    }

    void MapgenScene::create_moisture_mesh(MeshData* mesh, float z_scale)
    {
		std::vector<Vertex3PC> verts;
        Color c;
        for (const auto& p : graph.get_centers())
        {
            if (p->ocean)
            {
                c = ocean_color;
            }
            else if (p->water)
            {
                c = lake_color;
            }
            else
            {
                c = { 1.0f - p->moisture, p->moisture, 0.0f, 1.0f };
            }

            for (auto i = 1u; i < p->corners.size(); i++)
            {
				verts.push_back({ p->pos.x, p->elevation * z_scale, p->pos.y, c.r, c.g, c.b, c.a });
				verts.push_back({ p->corners[i]->pos.x, p->corners[i]->elevation * z_scale, p->corners[i]->pos.y, c.r, c.g, c.b, c.a });
				verts.push_back({ p->corners[i - 1]->pos.x, p->corners[i - 1]->elevation * z_scale, p->corners[i - 1]->pos.y, c.r, c.g, c.b, c.a });
            }

			verts.push_back({ p->pos.x, p->elevation * z_scale, p->pos.y, c.r, c.g, c.b, c.a });
			verts.push_back({ p->corners[0]->pos.x, p->corners[0]->elevation * z_scale, p->corners[0]->pos.y, c.r, c.g, c.b, c.a });
			verts.push_back({ p->corners[p->corners.size() - 1]->pos.x, p->corners[p->corners.size() - 1]->elevation * z_scale,
				p->corners[p->corners.size() - 1]->pos.y, c.r, c.g, c.b, c.a });
        }

		mesh->set_vertices(reinterpret_cast<GLfloat*>(verts.data()), verts.size());
    }

    void MapgenScene::create_biomes_mesh(MeshData* mesh, float z_scale)
    {
		std::vector<Vertex3PC> verts;
        Color c;
        for (const auto& p : graph.get_centers())
        {
            switch (p->biome)
            {
            case Ocean:
                c = ocean_color;
                break;
            case Lake:
                c = lake_color;
                break;
            case Ice:
            case Snow:
                c = ice_color;
                break;
            case Beach:
            case TemperateDesert:
            case SubtropicalDesert:
                c = land_color;
                break;
            case TemperateRainForest:
            case TropicalRainForest:
            case TropicalSeasonalForest:
                c = tropical_rf_color;
                break;
            case TemperateDeciduousForest:
            case Taiga:
                c = temperate_df_color;
                break;
            case Grassland:
                c = grassland_color;
                break;
            case Bare:
            case Shrubland:
                c = bare_color;
                break;
            case Scorched:
                c = scorched_color;
                break;
            case Marsh:
                c = marsh_color;
                break;
            case Tundra:
                c = tundra_color;
                break;
            }

            for (auto i = 1u; i < p->corners.size(); i++)
            {
				verts.push_back({ p->pos.x, p->elevation * z_scale, p->pos.y, c.r, c.g, c.b, c.a });
				verts.push_back({ p->corners[i]->pos.x, p->corners[i]->elevation * z_scale, p->corners[i]->pos.y, c.r, c.g, c.b, c.a });
				verts.push_back({ p->corners[i - 1]->pos.x, p->corners[i - 1]->elevation * z_scale, p->corners[i - 1]->pos.y, c.r, c.g, c.b, c.a });
            }
            
            verts.push_back({ p->pos.x, p->elevation * z_scale, p->pos.y, c.r, c.g, c.b, c.a });
            verts.push_back({ p->corners[0]->pos.x, p->corners[0]->elevation * z_scale, p->corners[0]->pos.y, c.r, c.g, c.b, c.a });
            verts.push_back({ p->corners[p->corners.size() - 1]->pos.x, p->corners[p->corners.size() - 1]->elevation * z_scale,
				p->corners[p->corners.size() - 1]->pos.y, c.r, c.g, c.b, c.a });
        }

		mesh->set_vertices(reinterpret_cast<GLfloat*>(verts.data()), verts.size());
    }

    void MapgenScene::create_edge_mesh(MeshData* mesh, float z_scale)
    {
        Color c{0.0f, 0.0f, 0.0f, 1.0f};
		std::vector<Vertex3PC> verts;
        for (const auto& edge : graph.get_edges())
        {
            verts.push_back({ edge->v0->pos.x, edge->v0->elevation * z_scale, edge->v0->pos.y, c.r, c.g, c.b, c.a });
            verts.push_back({ edge->v1->pos.x, edge->v1->elevation * z_scale, edge->v1->pos.y, c.r, c.g, c.b, c.a });
        }
		mesh->set_vertices(reinterpret_cast<GLfloat*>(verts.data()), verts.size());
    }

    void MapgenScene::create_river_mesh(MeshData* mesh, float z_scale)
    {
        Color c{0.0f, 0.0f, 1.0f, 1.0f};
		std::vector<Vertex3PC> verts;
        for (const auto& edge : graph.get_edges())
        {
			if (edge->river == 0)
				continue;
			verts.push_back({ edge->v0->pos.x, edge->v0->elevation * z_scale, edge->v0->pos.y, c.r, c.g, c.b, c.a });
			verts.push_back({ edge->v1->pos.x, edge->v1->elevation * z_scale, edge->v1->pos.y, c.r, c.g, c.b, c.a });
        }
		mesh->set_vertices(reinterpret_cast<GLfloat*>(verts.data()), verts.size());
    }

	void MapgenScene::render(void)
	{
		std::vector<Mesh*> meshes;
		meshes.push_back(game->get_debug_meshes());
		meshes.push_back(&object_meshes);
		meshes.push_back(&overlay_meshes);
		game->get_renderer()->render(meshes);
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
		dukat::Game3 app(settings);
		app.add_scene("main", std::make_unique<dukat::MapgenScene>(&app));
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