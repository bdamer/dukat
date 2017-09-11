// lightingapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "lightingapp.h"

namespace dukat
{
	LightingScene::LightingScene(Game3* game) : game(game), light(10.0f), animate_light(true)
	{
		// White Directional Light
		auto light0 = game->get_renderer()->get_light(Renderer3::dir_light_idx);
		light0->position = { 1.0f, 1.0f, 1.0f }; // light direction stored as position
		light0->ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
		light0->diffuse = { 0.4f, 0.4f, 0.4f, 1.0f };
		light0->specular = { 0.4f, 0.4f, 0.4f, 1.0f };
		
		// Red Point Light 
		auto light1 = game->get_renderer()->get_light(Renderer3::point_light_idx);
		light1->position = { 5.0f, 0.0f, 0.0f };
		light1->ambient = { 0.2f, 0.0f, 0.0f, 1.0f };
		light1->diffuse = { 0.5f, 0.0f, 0.0f, 1.0f };
		light1->specular = { 1.0f, 0.0f, 0.0f, 1.0f };
		light1->k0 = 1.0f; light1->k1 = 0.14f; light1->k2 = 0.07f; // Range ~20.0

		// Blue Point Light 
		auto light2 = game->get_renderer()->get_light(Renderer3::point_light_idx + 1);
		light2->position = { 0.0f, 10.0f, 0.0f };
		light2->ambient = { 0.0f, 0.2f, 0.0f, 1.0f };
		light2->diffuse = { 0.0f, 0.5f, 0.0f, 1.0f };
		light2->specular = { 0.0f, 1.0f, 0.0f, 1.0f };
		light2->k0 = 1.0f; light2->k1 = 0.14f; light2->k2 = 0.07f; // Range ~20.0

		// Green Point Light 
		auto light3 = game->get_renderer()->get_light(Renderer3::point_light_idx + 2);
		light3->position = { 0.0f, 0.0f, 7.0f };
		light3->ambient = { 0.0f, 0.0f, 0.2f, 1.0f };
		light3->diffuse = { 0.0f, 0.0f, 0.5f, 1.0f };
		light3->specular = { 0.0f, 0.0f, 1.0f, 1.0f };
		light3->k0 = 1.0f; light3->k1 = 0.14f; light3->k2 = 0.07f; // Range ~20.0

		auto settings = game->get_settings();
		auto camera = std::make_unique<OrbitCamera3>(game, Vector3{ 0.0f, 0.0f, 0.0f }, 10.0f, 0.0f, pi_over_four);
		camera->set_min_distance(1.0f);
		camera->set_max_distance(50.0f);
		camera->set_vertical_fov(settings.get_float("camera.fov"));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		object_meshes.stage = RenderStage::SCENE;
		object_meshes.visible = true;

		// Generate some meshes
		MeshBuilder3 mb3;
		auto box_mesh = object_meshes.create_instance();
		box_mesh->set_mesh(game->get_meshes()->put("box", mb3.build_cube()));
		box_mesh->set_program(game->get_shaders()->get_program("sc_lighting.vsh", "sc_lighting.fsh"));
		box_mesh->set_texture(game->get_textures()->get("blank.png"));
		box_mesh->set_material(mat_white_rubber);
		box_mesh->transform.position.x = 2.5f;
		
		auto sphere_mesh = object_meshes.create_instance();
		sphere_mesh->set_mesh(game->get_meshes()->put("sphere", mb3.build_sphere(32, 32)));
		sphere_mesh->set_program(game->get_shaders()->get_program("sc_lighting.vsh", "sc_lighting.fsh"));
		sphere_mesh->set_texture(game->get_textures()->get("blank.png"));
		sphere_mesh->set_material(mat_gold);
		sphere_mesh->transform.position.x = -2.5f;
		
		overlay_meshes.stage = RenderStage::OVERLAY;
		overlay_meshes.visible = true;

		auto info_text = game->create_text_mesh(1.0f / 20.0f);
		info_text->transform.position = { -1.0f, -0.8f, 0.0f };
		std::stringstream ss;
		ss << "<#white>"
			<< "<F1> Toggle Wirframe" << std::endl
			<< "<F2> Toggle light animation" << std::endl
			<< "<F11> Toggle Info" << std::endl
			<< std::endl;
		info_text->set_text(ss.str());
		info_text->transform.update();
		info_mesh = overlay_meshes.add_instance(std::move(info_text));

		game->set_controller(this);
	}

	bool LightingScene::handle_event(const SDL_Event& e)
	{
		switch (e.type)
		{
		case SDL_MOUSEWHEEL:
		{
			auto camera = game->get_renderer()->get_camera();
			camera->set_distance(camera->get_distance() - 2.0f * (float)e.wheel.y);
			break;
		}
		default:
			return false;
		}
		return true;
	}

	bool LightingScene::handle_keyboard(const SDL_Event & e)
	{
		Material mat;
		switch (e.key.keysym.sym)
		{
		case SDLK_F1:
			game->get_renderer()->toggle_wireframe();
			break;
		case SDLK_F2:
			animate_light = !animate_light;
			break;
		case SDLK_F11:
			info_mesh->visible = !info_mesh->visible;
			break;
		default:
			return false;
		}
		return true;
	}

	void LightingScene::update(float delta)
	{
		object_meshes.update(delta);
		overlay_meshes.update(delta);

		if (animate_light)
			light.update(delta, *game->get_renderer()->get_light(Renderer3::dir_light_idx));
	}

	void LightingScene::render(void)
	{
		std::vector<Mesh*> meshes;
		meshes.push_back(&object_meshes);
		meshes.push_back(&overlay_meshes);
		meshes.push_back(game->get_debug_meshes());
		game->get_renderer()->render(meshes);
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
		dukat::Game3 app(settings);
		app.add_scene("main", std::make_unique<dukat::LightingScene>(&app));
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