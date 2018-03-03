// mapgenapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "mapgenapp.h"

// TODO:
// - compute & visualize watersheds ... ongoing
// - select some more applicable colors for cells 
//		- base color is a function of altitude & humidity [i.e., cells near water are greener than cells without rivers or lakes]

// - rescale different elevation stages
// - water & watersheds
// - shade cells

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

	// Cell colors
	const Color desert_color = color_rgb(0xe4b871);
	const Color hill_color = color_rgb(0x9B7653);
	const Color mountain_color = color_rgb(0x837e7c);

	std::vector<Vector2> generate_point_set(int count, const AABB2& limits)
	{
		std::vector<Vector2> points(count);
		for (int i = 0; i < count; i++)
		{
			points[i] = Vector2::random(limits.min, limits.max);
		}
		return points;
	}

	MapgenScene::MapgenScene(Game3* game) : game(game), map(64, 64, 1.0f)
	{
		// Default values
		polygon_count = 500;
		render_mode = Overhead;
		map_mode = LandWater;
		noise_octaves = 3; // looks like at the current resolution we're not getting much past 3 iterations
		noise_frequency = 8.0f;
		noise_seed = 42;

		object_meshes.stage = RenderStage::SCENE;
		object_meshes.visible = true;

		// Create meshes to display diagram
		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3, offsetof(VertexPosCol, pos)));
		attr.push_back(VertexAttribute(Renderer::at_color, 4, offsetof(VertexPosCol, col)));

		line_mesh = overlay_meshes.create_instance();
		line_mesh->set_program(game->get_shaders()->get_program("sc_color.vsh", "sc_color.fsh"));
		line_mesh->set_mesh(game->get_meshes()->put("lines", std::make_unique<MeshData>(GL_LINES, 2048, 0, attr)));
		line_mesh->transform.position = { 0.0f, 0.01f, 0.0f };
		line_mesh->visible = false;

		grid_mesh = overlay_meshes.create_instance();
		grid_mesh->set_program(game->get_shaders()->get_program("sc_color.vsh", "sc_color.fsh"));
		grid_mesh->set_mesh(game->get_meshes()->put("grid", std::make_unique<MeshData>(GL_LINES, 2048, 2048, attr)));
		grid_mesh->transform.position = { 0.0f, 0.01f, 0.0f };

		fill_mesh = object_meshes.create_instance();
		fill_mesh->set_program(game->get_shaders()->get_program("sc_hexmap.vsh", "sc_hexmap.fsh"));
		fill_mesh->set_mesh(game->get_meshes()->put("triangulation", std::make_unique<MeshData>(GL_TRIANGLES, 2048, 2048, attr)));

		generate_map();
		switch_mode();

		// TODO: Need more render stages / make stages runtime objects
		overlay_meshes.stage = RenderStage::OVERLAY;
		overlay_meshes.visible = true;

		auto info_text = game->create_text_mesh(1.0f / 20.0f);
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
		info_mesh->visible = false;

		game->set_controller(this);
	}

	void MapgenScene::generate_map(void)
	{
		map.reset();

		// TODO: look into combining noise
		// Lower frequencies are looking quite good - 1-4
		// so maybe combine 1-2 passes to add some varietay!
		//		map.noise_pass(32.0f, 10, 42);
		logger << "Generate map: " << noise_frequency << " - " << noise_octaves << " - " << noise_seed << std::endl;
		map.elevation_pass(noise_frequency, noise_octaves, noise_seed);
		map.moisture_pass();

		// AABB2 limits(Vector2({ -1.0f, -1.0f }), Vector2({ 1.0f, 1.0f }));
		// auto points = generate_point_set(polygon_count, limits);
		// graph.from_points(points);
		// graph.generate();
		// create_water_land_mesh(fill_mesh->get_mesh());
		// create_edge_mesh(line_mesh->get_mesh());
		create_hex_mesh(fill_mesh->get_mesh(), 2.5f);
		create_river_mesh(line_mesh->get_mesh(), 2.5f);
		create_grid_mesh(grid_mesh->get_mesh(), 2.5f);
	}

	void MapgenScene::switch_mode(void)
	{
		float z_scale;
		auto settings = game->get_settings();
		if (render_mode == Overhead)
		{
			auto camera = std::make_unique<FixedCamera3>(game, Vector3{ 0.0f, 25.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f }, -Vector3::unit_z);
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
			auto camera = std::make_unique<FixedCamera3>(game, Vector3{ 0.0f, 10.0f, 10.0f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3::unit_y);
			camera->set_vertical_fov(settings.get_float("camera.fov"));
			camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
			camera->refresh();
			game->get_renderer()->set_camera(std::move(camera));
			z_scale = 0.25f;
		}
	}

	void MapgenScene::handle_event(const SDL_Event& e)
	{
		switch (e.type)
		{
		case SDL_MOUSEWHEEL:
		{
			auto camera = game->get_renderer()->get_camera();
			auto dist = camera->get_distance() - 2.0f * (float)e.wheel.y;
			camera->set_distance(std::max(5.0f, dist));
			break;
		}
		}
	}

	void MapgenScene::handle_keyboard(const SDL_Event & e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;

		case SDLK_COMMA:
			noise_octaves--;
			break;
		case SDLK_PERIOD:
			noise_octaves++;
			break;

		case SDLK_r: // new random seed
			noise_seed = static_cast<uint32_t>(time(nullptr));
			generate_map();
			break;
		case SDLK_g: // generate map
			generate_map();
			break;
		case SDLK_c: // camera mode 
			render_mode = render_mode == Overhead ? Perspective : Overhead;
			switch_mode();
			break;
		case SDLK_F10:
			game->get_renderer()->toggle_wireframe();
			break;
		case SDLK_F11:
			info_mesh->visible = !info_mesh->visible;
			break;
		}
	}

	void MapgenScene::update(float delta)
	{
		auto dev = game->get_devices()->active;
		auto offset = 10.0f * delta * (-dev->ly * Vector3::unit_z + dev->lx * Vector3::unit_x);
		auto cam = dynamic_cast<FixedCamera3*>(game->get_renderer()->get_camera());
		cam->move(offset);

		object_meshes.update(delta);
		overlay_meshes.update(delta);
	}

	/*
	void MapgenScene::create_water_land_mesh(MeshData* mesh, float z_scale)
    {
		std::vector<VertexPosCol> verts;
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
        std::vector<VertexPosCol> verts;
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
		std::vector<VertexPosCol> verts;
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
		std::vector<VertexPosCol> verts;
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
		std::vector<VertexPosCol> verts;
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
		std::vector<VertexPosCol> verts;
        for (const auto& edge : graph.get_edges())
        {
			if (edge->river == 0)
				continue;
			verts.push_back({ edge->v0->pos.x, edge->v0->elevation * z_scale, edge->v0->pos.y, c.r, c.g, c.b, c.a });
			verts.push_back({ edge->v1->pos.x, edge->v1->elevation * z_scale, edge->v1->pos.y, c.r, c.g, c.b, c.a });
        }
		mesh->set_vertices(reinterpret_cast<GLfloat*>(verts.data()), verts.size());
    }
	*/

	void MapgenScene::create_grid_mesh(MeshData* mesh, float z_scale)
	{
		std::vector<VertexPosCol> verts;
		std::vector<GLushort> indices;

		for (auto& q : map.corners)
		{
			verts.push_back({ q.pos.x, z_scale * q.elevation, q.pos.y, 0.4f, 0.4f, 0.4f, 0.5f });
		}

		for (auto y = 0; y < map.corners_height; y++)
		{
			for (auto x = 0; x < map.corners_width; x++)
			{
				auto idx = y * map.corners_width + x;
				// skip ghost corner in last row
				if (y == map.corners_height - 1)
				{
					if ((map.get_height() % 2 == 0) && (x == 0)) // even number of cell rows -> ghost on 1st
						continue;
					if ((map.get_height() % 2 == 1) && (x == map.corners_width - 1)) // odd number of cell rows -> ghost on last
						continue;
				}

				if ((y > 0 && x < map.corners_width - 1) || (y == 0 && x < map.corners_width - 2))
				{
					// horizontal line
					indices.push_back(idx);
					indices.push_back(idx + 1);
				}

				if (y < map.corners_height - 1 && ((x + y) % 2 == 0))
				{
					// vertical line
					indices.push_back(idx);
					indices.push_back(idx + map.corners_width);
				}
			}
		}

		mesh->set_vertices(reinterpret_cast<GLfloat*>(verts.data()), verts.size());
		mesh->set_indices(reinterpret_cast<GLushort*>(indices.data()), indices.size());
	}

	void MapgenScene::create_hex_mesh(MeshData* mesh, float z_scale)
	{
		std::vector<VertexPosCol> verts;
		std::vector<GLushort> indices;

		GLushort vidx = 0;
		for (auto i = 0; i < map.get_width() * map.get_height(); i++)
		{
			auto cell = map.get_cell(i);

			verts.push_back({ cell->pos.x, z_scale * cell->elevation, cell->pos.y, 
				cell->elevation, cell->water ? 1.0f : 0.0f, cell->moisture, 1.0f });

			for (auto q : cell->corners)
			{
				verts.push_back({ q->pos.x, z_scale * q->elevation, q->pos.y, 
					q->elevation, q->water ? 1.0f : 0.0f, q->moisture, 1.0f });
			}

			indices.insert(indices.end(), {
				static_cast<GLushort>(vidx), static_cast<GLushort>(vidx + 1), static_cast<GLushort>(vidx + 2),
				static_cast<GLushort>(vidx), static_cast<GLushort>(vidx + 2), static_cast<GLushort>(vidx + 3),
				static_cast<GLushort>(vidx), static_cast<GLushort>(vidx + 3), static_cast<GLushort>(vidx + 4),
				static_cast<GLushort>(vidx), static_cast<GLushort>(vidx + 4), static_cast<GLushort>(vidx + 5),
				static_cast<GLushort>(vidx), static_cast<GLushort>(vidx + 5), static_cast<GLushort>(vidx + 6),
				static_cast<GLushort>(vidx), static_cast<GLushort>(vidx + 6), static_cast<GLushort>(vidx + 1)
			});

			vidx += 7;
		}

		mesh->set_vertices(reinterpret_cast<GLfloat*>(verts.data()), verts.size());
		mesh->set_indices(reinterpret_cast<GLushort*>(indices.data()), indices.size());
	}

	void MapgenScene::create_river_mesh(MeshData* mesh, float z_scale)
	{
		Color c{ 0.0f, 0.0f, 1.0f, 1.0f };
		std::vector<VertexPosCol> verts;
		for (const auto& river :map.rivers)
		{
			HexMap::Corner* last = nullptr;
			for (auto corner : river.corners)
			{
				if (last != nullptr)
				{
					verts.push_back({ last->pos.x, z_scale * last->elevation, last->pos.y, c.r, c.g, c.b, c.a });
					verts.push_back({ corner->pos.x, z_scale * corner->elevation, corner->pos.y, c.r, c.g, c.b, c.a });
				}
				last = corner;
			}
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
		dukat::logger << "Application failed with error." << std::endl << e.what() << std::endl;
		return -1;
	}
	return 0;
}