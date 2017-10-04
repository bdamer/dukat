#include "stdafx.h"
#include "titlescene.h"
#include "textbutton.h"
#include <dukat/dukat.h>

namespace dukat
{
	TitleScene::TitleScene(Game3* game3) : game(game3)
	{
		overlay_meshes.stage = RenderStage::OVERLAY;
		overlay_meshes.visible = true;

		auto title_text = game->create_text_mesh(1.0f / 10.0f);
		title_text->transform.position = { -0.75f, 0.25f, 0.0f };
		title_text->set_text("<#red>Title screen</>");
		overlay_meshes.add_instance(std::move(title_text));

		auto start_text = game->create_text_mesh(1.0f / 20.0f);
		start_text->transform.position = { -1.0f, 0.0f, 0.0f };
		start_button = std::make_unique<TextButton>(start_text.get());
		start_button->set_text("Start Game");
		start_button->set_index(0);
		start_button->set_trigger([&](void) {
			game->push_scene("game");
		});
		overlay_meshes.add_instance(std::move(start_text));

		auto options_text = game->create_text_mesh(1.0f / 20.0f);
		options_text->transform.position = { -1.0f, -0.1f, 0.0f };
		options_button = std::make_unique<TextButton>(options_text.get());
		options_button->set_text("Options");
		options_button->set_index(1);
		options_button->set_trigger([&](void) {
			game->push_scene("options");
		});
		overlay_meshes.add_instance(std::move(options_text));

		auto quit_text = game->create_text_mesh(1.0f / 20.0f);
		quit_text->transform.position = { -1.0f, -0.2f, 0.0f };
		quit_button = std::make_unique<TextButton>(quit_text.get());
		quit_button->set_text("Quit");
		quit_button->set_index(2);
		quit_button->set_trigger([&](void) {
			game->set_done(true);
		});
		overlay_meshes.add_instance(std::move(quit_text));
	}	

	void TitleScene::activate(void)
	{ 
		auto settings = game->get_settings();
		auto camera = std::make_unique<FixedCamera3>(game, Vector3{ 0.0f, 0.0f, 2.5f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3::unit_y);
		camera->set_vertical_fov(settings.get_float("camera.fov"));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		game->set_controller(this);

		game->get_ui()->add_control(start_button.get());
		game->get_ui()->add_control(options_button.get());
		game->get_ui()->add_control(quit_button.get());
	}

	void TitleScene::deactivate(void)
	{
		game->get_ui()->remove_control(start_button.get());
		game->get_ui()->remove_control(options_button.get());
		game->get_ui()->remove_control(quit_button.get());
	}

	void TitleScene::update(float delta)
	{
		overlay_meshes.update(delta);
	}

	void TitleScene::handle_keyboard(const SDL_Event& e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_SPACE:
		case SDLK_RETURN:
			game->get_ui()->trigger_focus();
			break;
		case SDLK_UP:
			game->get_ui()->prev_control();
			break;
		case SDLK_DOWN:
			game->get_ui()->next_control();
			break;
		}
	}

	void TitleScene::render(void)
	{
		auto renderer = game->get_renderer();
		std::vector<Mesh*> meshes;
		meshes.push_back(game->get_debug_meshes());
		meshes.push_back(&overlay_meshes);
		renderer->render(meshes);
	}
}