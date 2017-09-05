// modelviewer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "modelviewer.h"

namespace dukat
{
	void Game::init(void)
	{
		Game3::init();

		renderer->disable_effects();

		// White Directional Light
		auto light0 = renderer->get_light(Renderer3::dir_light_idx);
		light0->position = { 0.5f, 1.0f, 0.0f }; // light direction stored as position
		light0->ambient = { 0.25f, 0.25f, 0.25f, 1.0f };
		light0->diffuse = { 0.5f, 0.5f, 0.5f, 1.0f };
		light0->specular = { 0.4f, 0.4f, 0.4f, 1.0f };
		
		camera_target = { 0.0f, 0.0f, 0.0f };

		auto camera = std::make_unique<OrbitCamera3>(this, camera_target, 50.0f, 0.0f, pi_over_four);
		camera->set_min_distance(5.0f);
		camera->set_max_distance(100.0f);
		camera->set_vertical_fov(settings.get_float("camera.fov"));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		renderer->set_camera(std::move(camera));		

		overlay_meshes.stage = RenderStage::OVERLAY;
		overlay_meshes.visible = true;

		auto info_text = create_text_mesh(1.0f / 20.0f);
		info_text->transform.position = { -1.5f, -0.5f, 0.0f };
		std::stringstream ss;
		ss << "<#white>"
			<< "<,.> Toggle Meshes" << std::endl
			<< "<L> Show all Meshes" << std::endl
			<< "<[]> Change scale" << std::endl
			<< "<F1> Toggle Wirframe" << std::endl
			<< "<F2> Toggle Lighting" << std::endl
			<< "<F11> Toggle Info" << std::endl
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

	void Game::load_model(const std::string& filename)
	{
		std::stringstream ss; 
		ss << settings.get_string("resources.models") << "/" << filename;
	
		auto is = std::fstream(ss.str(), std::fstream::in | std::fstream::binary);
		if (!is)
		{
			throw std::runtime_error("Could not open file.");
		}

		auto ext = get_extension(filename);
		if (ext == "ms3d")
		{
			MS3DModel ms3d;
			is >> ms3d;
			model = ms3d.convert();
		}
		else
		{
			// Load dukat native model
			model = std::make_unique<Model3>();
			is >> *model;
		}

		object_meshes = build_mesh_group(this, *model);
		object_meshes->stage = RenderStage::SCENE;
		object_meshes->visible = true;
	}

	void Game::save_model(const std::string& filename)
	{
		logger << "Saving model as: " << filename << std::endl;
		auto os = std::fstream(filename, std::fstream::out | std::fstream::binary);
		if (!os)
			throw std::runtime_error("Could not open file");
		os << *model;
		os.close();
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
		Material mat;
		switch (e.key.keysym.sym)
		{
		case SDLK_F1:
			renderer->toggle_wireframe();
			break;
		case SDLK_F2:
			{
				enable_lighting = !enable_lighting;
				for (auto i = 0u; i < object_meshes->size(); i++)
				{
					if (enable_lighting)
					{
						object_meshes->get_instance(i)->set_program(shader_cache->get_program("sc_lighting.vsh", "sc_lighting.fsh"));
					}
  					else
					{
						object_meshes->get_instance(i)->set_program(shader_cache->get_program("sc_texture.vsh", "sc_texture.fsh"));
					}
				}
			}
			break;
		case SDLK_F5: // save current model
			save_model("model.mod");
			break;
		case SDLK_F6: // reload from assets path
			load_model("sloop.mod");
			break;
		case SDLK_F11:
			info_mesh->visible = !info_mesh->visible;
			break;
		// Cycle through mesh instances
		case SDLK_PERIOD: // show next
		{
			const auto count = object_meshes->size();
			selected_mesh++;
			if (selected_mesh >= count)
				selected_mesh = 0;
			logger << "Showing mesh " << selected_mesh << ": " << 
				object_meshes->get_instance(selected_mesh)->get_name() << std::endl;
			for (auto i = 0u; i < count; i++)
			{
				object_meshes->get_instance(i)->visible = selected_mesh == i;
			}
		}
		break;
		case SDLK_COMMA: // show previous
		{
			const auto count = object_meshes->size();
			selected_mesh--;
			if (selected_mesh < 0)
				selected_mesh = (int)count - 1;
			logger << "Showing mesh " << selected_mesh << ": " << 
				object_meshes->get_instance(selected_mesh)->get_name() << std::endl;
			for (auto i = 0u; i < count; i++)
			{
				object_meshes->get_instance(i)->visible = selected_mesh == i;
			}
		}
		break;
        case SDLK_l: // show all
		{
			selected_mesh = -1;
			for (auto i = 0u; i < object_meshes->size(); i++)
			{
				object_meshes->get_instance(i)->visible = true;
			}
		}
		break;
		case SDLK_LEFTBRACKET:
		{
			for (auto i = 0u; i < object_meshes->size(); i++)
			{
				auto inst = object_meshes->get_instance(i);
				inst->transform.scale -= Vector3{ 1.0f, 1.0f, 1.0f };
			}
		}
		break;
		case SDLK_RIGHTBRACKET:
		{
			for (auto i = 0u; i < object_meshes->size(); i++)
			{
				auto inst = object_meshes->get_instance(i);
				inst->transform.scale += Vector3{ 1.0f, 1.0f, 1.0f };
			}
		}
		break;
		default:
			Game3::handle_keyboard(e);
		}
	}

	void Game::update(float delta)
	{
		Game3::update(delta);
		object_meshes->update(delta);
		overlay_meshes.update(delta);
		debug_meshes.update(delta);

		light.update(delta, *renderer->get_light(Renderer3::dir_light_idx));
	}

	void Game::render(void)
	{
		std::vector<Mesh*> meshes;
		meshes.push_back(&debug_meshes);
		meshes.push_back(object_meshes.get());
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
		std::string config = "../assets/modelviewer.ini";
		dukat::Settings settings(config);
		dukat::Game app(settings);

		if (argc > 1)
		{
			app.load_model(argv[1]);
		}
		else
		{
			app.load_model("sloop.mod");
		}

		return app.run();
	}
	catch (const std::exception& e)
	{
		dukat::logger << "Application failed with error." << std::endl << e.what() << std::endl;
		return -1;
	}
	return 0;
}