// lightingapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "lightingapp.h"

#include <dukat/devicemanager.h>
#include <dukat/inputdevice.h>
#include <dukat/fixedcamera3.h>
#include <dukat/log.h>
#include <dukat/mathutil.h>
#include <dukat/meshbuilder3.h>
#include <dukat/orbitcamera3.h>
#include <dukat/settings.h>

namespace dukat
{
	void Game::init(void)
	{
		Game3::init();

		// White Directional Light
		auto light0 = renderer->get_light(Renderer3::dir_light_idx);
		light0->position = { 1.0f, 1.0f, 1.0f }; // light direction stored as position
		light0->ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
		light0->diffuse = { 0.4f, 0.4f, 0.4f, 1.0f };
		light0->specular = { 0.4f, 0.4f, 0.4f, 1.0f };
		
		// Red Point Light 
		auto light1 = renderer->get_light(Renderer3::point_light_idx);
		light1->position = { 5.0f, 0.0f, 0.0f };
		light1->ambient = { 0.2f, 0.0f, 0.0f, 1.0f };
		light1->diffuse = { 0.5f, 0.0f, 0.0f, 1.0f };
		light1->specular = { 1.0f, 0.0f, 0.0f, 1.0f };
		light1->k0 = 1.0f; light1->k1 = 0.14f; light1->k2 = 0.07f; // Range ~20.0

		// Blue Point Light 
		auto light2 = renderer->get_light(Renderer3::point_light_idx + 1);
		light2->position = { 0.0f, 10.0f, 0.0f };
		light2->ambient = { 0.0f, 0.2f, 0.0f, 1.0f };
		light2->diffuse = { 0.0f, 0.5f, 0.0f, 1.0f };
		light2->specular = { 0.0f, 1.0f, 0.0f, 1.0f };
		light2->k0 = 1.0f; light2->k1 = 0.14f; light2->k2 = 0.07f; // Range ~20.0

		// Green Point Light 
		auto light3 = renderer->get_light(Renderer3::point_light_idx + 2);
		light3->position = { 0.0f, 0.0f, 7.0f };
		light3->ambient = { 0.0f, 0.0f, 0.2f, 1.0f };
		light3->diffuse = { 0.0f, 0.0f, 0.5f, 1.0f };
		light3->specular = { 0.0f, 0.0f, 1.0f, 1.0f };
		light3->k0 = 1.0f; light3->k1 = 0.14f; light3->k2 = 0.07f; // Range ~20.0

		auto camera = std::make_unique<OrbitCamera3>(this, Vector3{ 0.0f, 0.0f, 0.0f }, 10.0f, 0.0f, pi_over_four);
		camera->set_min_distance(1.0f);
		camera->set_max_distance(50.0f);
		camera->set_vertical_fov(settings.get_float("camera.fov"));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		renderer->set_camera(std::move(camera));

		object_meshes.stage = RenderStage::SCENE;
		object_meshes.mat_model.identity();
		object_meshes.visible = true;

		// Generate some meshes
		MeshBuilder3 mb3;
		auto box_mesh = object_meshes.create_instance();
		box_mesh->set_mesh(mesh_cache->put("box", mb3.build_cube()));
		box_mesh->set_program(shader_cache->get_program("sc_lighting.vsh", "sc_lighting.fsh"));
		box_mesh->set_texture(texture_cache->get("blank.png"));
		box_mesh->set_material(mat_white_rubber);
		box_mesh->transform.position.x = 2.5f;
		
		auto sphere_mesh = object_meshes.create_instance();
		sphere_mesh->set_mesh(mesh_cache->put("sphere", mb3.build_sphere(32, 32)));
		sphere_mesh->set_program(shader_cache->get_program("sc_lighting.vsh", "sc_lighting.fsh"));
		sphere_mesh->set_texture(texture_cache->get("blank.png"));
		sphere_mesh->set_material(mat_gold);
		sphere_mesh->transform.position.x = -2.5f;
		
		overlay_meshes.stage = RenderStage::OVERLAY;
		overlay_meshes.visible = true;
		overlay_meshes.mat_model.identity();

		auto info_text = create_text_mesh(1.0f / 20.0f);
		info_text->transform.position = { -1.5f, -0.5f, 0.0f };
		std::stringstream ss;
		ss << "<#magenta>"
			<< "<F1> Toggle Wirframe" << std::endl
			<< "<F11> Toggle Info" << std::endl
			<< std::endl;
		info_text->set_text(ss.str());
		info_text->transform.update();
		info_mesh = overlay_meshes.add_instance(std::move(info_text));
		info_mesh->visible = false;
		
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
		Material mat;
		switch (e.key.keysym.sym)
		{
		case SDLK_F1:
			renderer->toggle_wireframe();
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
		object_meshes.update(delta);
		overlay_meshes.update(delta);
		debug_meshes.update(delta);

		light.update(delta, *renderer->get_light(Renderer3::dir_light_idx));
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
		std::string config = "../assets/lighting.ini";
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