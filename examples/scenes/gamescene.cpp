#include "stdafx.h"
#include "gamescene.h"
#include <dukat/dukat.h>

namespace dukat
{
	GameScene::GameScene(Game3* game) : game(game), light(25.0f), enable_lighting(false)
	{
		MeshBuilder3 mb3;
		std::stringstream ss;

		camera_target = { 0.0f, 0.0f, 0.0f };

		// White Directional Light
		auto light0 = game->get_renderer()->get_light(Renderer3::dir_light_idx);
		light0->position = { 0.5f, 1.0f, 0.0f }; // light direction stored as position
		light0->ambient = { 0.25f, 0.25f, 0.25f, 1.0f };
		light0->diffuse = { 0.5f, 0.5f, 0.5f, 1.0f };
		light0->specular = { 0.4f, 0.4f, 0.4f, 1.0f };

		scene_meshes.stage = RenderStage::SCENE;
		scene_meshes.visible = true;

		auto box = scene_meshes.create_instance();
		box->set_mesh(game->get_meshes()->put("box", mb3.build_cube_single_face()));
		box->set_texture(game->get_textures()->get("box01_1024.png"));
		box->set_program(game->get_shaders()->get_program("sc_lighting.vsh", "sc_lighting.fsh"));
		box->transform.scale = Vector3{ 2.0f, 2.0f, 2.0f };

		overlay_meshes.stage = RenderStage::OVERLAY;
		overlay_meshes.visible = true;

		auto info_text = game->create_text_mesh(1.0f / 20.0f);
		info_text->transform.position = { -1.5f, -0.5f, 0.0f };
		ss.str("");
		ss << "<#white>"
			<< "<F1> Toggle Wirframe" << std::endl
			<< "<F11> Toggle Info" << std::endl
			<< "<ESC> Return to Title" << std::endl
			<< std::endl;
		info_text->set_text(ss.str());
		info_mesh = overlay_meshes.add_instance(std::move(info_text));
	}

	void GameScene::activate(void)
	{
		auto settings = game->get_settings();
		auto camera = std::make_unique<OrbitCamera3>(game, camera_target, 10.0f, 0.0f, pi_over_four);
		camera->set_min_distance(5.0f);
		camera->set_max_distance(100.0f);
		camera->set_vertical_fov(game->get_settings().get_float("camera.fov"));
		camera->set_clip(game->get_settings().get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		game->set_controller(this);
	}

	void GameScene::update(float delta)
	{
		scene_meshes.update(delta);
		overlay_meshes.update(delta);
		light.update(delta, *game->get_renderer()->get_light(Renderer3::dir_light_idx));
	}

	bool GameScene::handle_event(const SDL_Event& e)
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

	bool GameScene::handle_keyboard(const SDL_Event& e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_F1:
			game->get_renderer()->toggle_wireframe();
			break;
		case SDLK_F11:
			info_mesh->visible = !info_mesh->visible;
			break;
		case SDLK_ESCAPE:
			game->pop_scene();
			break;
		default:
			return false;
		}
		return true;
	}

	void GameScene::render(void)
	{
		std::vector<Mesh*> meshes;
		meshes.push_back(game->get_debug_meshes());
		meshes.push_back(&scene_meshes);
		meshes.push_back(&overlay_meshes);
		game->get_renderer()->render(meshes);
	}
}