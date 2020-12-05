// heatmapapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "heatmapapp.h"
#include "heatmap.h"

namespace dukat
{
	HeatmapScene::HeatmapScene(Game3* game) : game(game), multiplier(1), animate_light(true)
	{ 
		auto settings = game->get_settings();

		// Yellow Directional Light
		auto light0 = game->get_renderer()->get_light(Renderer3::dir_light_idx);
		light0->position = { 0.0f, -0.5f, 0.5f }; // light direction stored as position
		light0->ambient = { 0.2f, 0.1f, 0.05f, 1.0f };
		light0->diffuse = { 0.5f, 0.43f, 0.1f, 1.0f };
		light = std::make_unique<OrbitalLight>(600.0f);

		// Observer
		MeshBuilder3 builder3;
		game->get_meshes()->put("observer", builder3.build_sphere(16, 16));
		observer_mesh = object_meshes.create_instance();
		observer_mesh->set_mesh(game->get_meshes()->get("observer"));
		observer_mesh->set_program(game->get_shaders()->get_program("sc_texture.vsh", "sc_texture.fsh"));
		observer_mesh->set_texture(game->get_textures()->get("white.png"));
		Material m;
		m.ambient = { 1.0f, 1.0f, 0.0f, 1.0f };
		observer_mesh->set_material(m);

		const int map_size = 256;
		camera_target.x = map_size / 2;
		camera_target.z = map_size / 2;

		auto camera = std::make_unique<OrbitCamera3>(game, camera_target, 50.0f, 0.0f, pi_over_four);
		camera->set_min_distance(5.0f);
		camera->set_max_distance(100.0f);
		camera->set_vertical_fov(settings.get_float("camera.fov"));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));		

		object_meshes.stage = RenderStage::Scene;
		object_meshes.visible = true;

		heatmap = std::make_unique<HeatMap>(game, map_size, 100.0f);
		//heatmap->set_tile_spacing(5);
		DiamondSquareGenerator gen(42);
		gen.set_range(0.0f, 0.1f);
		gen.set_roughness(10.0f);
		//heatmap->generate(gen);
		std::srand(28);
		heatmap->add_emitters(24, 36);
//		heatmap->add_emitters(12, 64);

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
		info_text->transform.position = { -1.5f, -0.5f, 0.0f };
		std::stringstream ss;
		ss << "<,.> Change Sim Speed" << std::endl
			<< "<e> Toggle Emitters" << std::endl
			<< "<F1> Toggle Wirframe" << std::endl
			<< "<F2> Toggle Light Animation" << std::endl
			<< "<F5> Save Heatmap" << std::endl
			<< "<F6> Load Heatmap" << std::endl
			<< "<F11> Toggle Info" << std::endl
			<< std::endl;
		info_text->set_text(ss.str());
		info_text->transform.update();
		info_mesh = overlay_meshes.add_instance(std::move(info_text));

		auto multiplier_text = game->create_text_mesh();
		multiplier_text->set_size(1.0f / 20.0f);
		multiplier_text->transform.position = { -1.6f, 0.85f, 0.0f };
		ss.str("");
		ss << "<#f20884>Speed " << multiplier << "x</>";
		multiplier_text->set_text(ss.str());
		multiplier_text->transform.update();
		mult_mesh = static_cast<TextMeshInstance*>(overlay_meshes.add_instance(std::move(multiplier_text)));

		game->set_controller(this);
	}

	void HeatmapScene::handle_event(const SDL_Event& e)
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

	void HeatmapScene::handle_keyboard(const SDL_Event & e)
	{
		Material mat;
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;
		case SDLK_COMMA:
			{
				multiplier = std::max(1, multiplier / 2);
				std::stringstream ss;
				ss << "<#f20884>Speed " << multiplier << "x</>";
				mult_mesh->set_text(ss.str());
			}
			break;
		case SDLK_PERIOD:
			{
				multiplier = std::min(64, multiplier * 2);
				std::stringstream ss;
				ss << "<#f20884>Speed " << multiplier << "x</>";
				mult_mesh->set_text(ss.str());
			}
			break;
		case SDLK_e:
			heatmap->toggle_emitters();
			break;
		case SDLK_F1:
			game->get_renderer()->toggle_wireframe();
			break;
		case SDLK_F2:
			animate_light = !animate_light;
			break;
		case SDLK_F5:
			heatmap->save("heatmap.png");
			break;
		case SDLK_F6:
			heatmap->load("heatmap.png");
			break;
		case SDLK_F11:
			info_mesh->visible = !info_mesh->visible;
			break;
		}
	}

	void HeatmapScene::update(float delta)
	{
		if (animate_light)
		{
			auto l0 = game->get_renderer()->get_light(Renderer3::dir_light_idx);
			light->update(delta * (float)multiplier, *l0);
			// height map always starts at 0 / 0 for lower left corner of inner level
			observer_mesh->transform.position = Vector3{128.0f, 0.0f, 128.0f} - 128.0f * l0->position;
		}
		
		heatmap->update(delta * (float)multiplier);

		auto dev = game->get_devices()->active;
		auto cam = dynamic_cast<OrbitCamera3*>(game->get_renderer()->get_camera());
		camera_target += 10.0f * delta * (dev->ly * cam->transform.dir
				+ dev->lx * cam->transform.right);
		camera_target.y = 0.0;
		cam->set_look_at(camera_target);

		object_meshes.update(delta);
		overlay_meshes.update(delta);
	}

	void HeatmapScene::render(void)
	{
		std::vector<Mesh*> meshes;
		meshes.push_back(&object_meshes);
		meshes.push_back(heatmap.get());
		meshes.push_back(&overlay_meshes);
		meshes.push_back(game->get_debug_meshes());
		game->get_renderer()->render(meshes);
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/heatmap.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game3 app(settings);
		app.add_scene("main", std::make_unique<dukat::HeatmapScene>(&app));
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