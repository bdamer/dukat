#include "stdafx.h"
#include "dialogscene.h"

namespace dukat
{
    DialogScene::DialogScene(Game3* game3) : game(game3)
	{
		overlay_meshes.stage = RenderStage::OVERLAY;
		overlay_meshes.visible = true;

		auto title_text = game->create_text_mesh(1.0f / 10.0f);
		title_text->transform.position = { -0.75f, 0.25f, 0.0f };
		title_text->set_text("<#red>Options screen</>");
		overlay_meshes.add_instance(std::move(title_text));

		auto fullscreen_text = game->create_text_mesh(1.0f / 20.0f);
		fullscreen_text->transform.position = { -1.0f, 0.0f, 0.0f };
		fullscreen_button = std::make_unique<TextButton>(fullscreen_text.get());
		fullscreen_button->set_text("Fullscreen");
		fullscreen_button->set_index(0);
		fullscreen_button->set_trigger([&](void) {
			bool fullscreen = !game->get_window()->is_fullscreen();
			game->get_window()->set_fullscreen(fullscreen);
			fullscreen_button->set_text(fullscreen ? "Windowed" : "Fullscreen");
		});
		overlay_meshes.add_instance(std::move(fullscreen_text));

		auto return_text = game->create_text_mesh(1.0f / 20.0f);
		return_text->transform.position = { -1.0f, -0.1f, 0.0f };
		return_button = std::make_unique<TextButton>(return_text.get());
		return_button->set_text("Back");
		return_button->set_index(1);
		return_button->set_trigger([&](void) {
			game->pop_scene();
		});
		overlay_meshes.add_instance(std::move(return_text));
	}

	void DialogScene::activate(void)
	{ 
		auto settings = game->get_settings();
		auto camera = std::make_unique<FixedCamera3>(game, Vector3{ 0.0f, 0.0f, 2.5f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3::unit_y);
		camera->set_vertical_fov(settings.get_float("camera.fov"));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		game->set_controller(this);

		game->get<UIManager>()->add_control(fullscreen_button.get());
		game->get<UIManager>()->add_control(return_button.get());
	}

	void DialogScene::deactivate(void)
	{
		game->get<UIManager>()->remove_control(fullscreen_button.get());
		game->get<UIManager>()->remove_control(return_button.get());
	}

	void DialogScene::update(float delta)
	{
		overlay_meshes.update(delta);
	}

	void DialogScene::handle_keyboard(const SDL_Event& e)
	{
		switch (e.key.keysym.sym)
		{
        case SDLK_ESCAPE:
            game->pop_scene();
            break;
		case SDLK_SPACE:
		case SDLK_RETURN:
			game->get<UIManager>()->trigger_focus();
			break;
		case SDLK_UP:
			game->get<UIManager>()->prev_control();
			break;
		case SDLK_DOWN:
			game->get<UIManager>()->next_control();
			break;
		}
	}

	void DialogScene::render(void)
	{
		auto renderer = game->get_renderer();
		std::vector<Mesh*> meshes;
		meshes.push_back(game->get_debug_meshes());
		meshes.push_back(&overlay_meshes);
		renderer->render(meshes);
	}
}