#include "stdafx.h"
#include "titlescene.h"
#include <dukat/dukat.h>

namespace dukat
{
	constexpr auto title_size = 1.0f / 10.0f;
	constexpr auto text_size = 1.0f / 20.0f;

	TitleScene::TitleScene(Game3* game3) : game(game3)
	{
		overlay_meshes.stage = RenderStage::Overlay;
		overlay_meshes.visible = true;

		create_controls();
	}	

	void TitleScene::activate(void)
	{ 
		auto settings = game->get_settings();
		auto camera = std::make_unique<FixedCamera3>(game, Vector3{ 0.0f, 0.0f, 2.5f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3::unit_y);
		camera->set_vertical_fov(settings.get_float(settings::video_camera_fov));
		camera->set_clip(settings.get_float(settings::video_camera_nearclip), settings.get_float(settings::video_camera_farclip));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		game->set_controller(this);

		game->get<UIManager>()->add_control(fractals_button.get());
		game->get<UIManager>()->add_control(ripples_button.get());
		game->get<UIManager>()->add_control(caustics_button.get());
		game->get<UIManager>()->add_control(quit_button.get());
		game->get<UIManager>()->first_control();
	}

	void TitleScene::deactivate(void)
	{
		game->get<UIManager>()->remove_control(fractals_button.get());
		game->get<UIManager>()->remove_control(ripples_button.get());
		game->get<UIManager>()->remove_control(caustics_button.get());
		game->get<UIManager>()->remove_control(quit_button.get());
	}

	void TitleScene::create_controls(void)
	{
		auto title_text = game->create_text_mesh();
		title_text->set_size(title_size);
		title_text->transform.position = { 0.0f, 0.25f, 0.0f };
		title_text->set_text("<#dd0907>Framebuffer Effects</>");
		title_text->halign = TextMeshInstance::Align::Center;
		overlay_meshes.add_instance(std::move(title_text));

		const auto x_offset = -1.f;
		auto y_offset = 0.0f;
		auto fractals_text = game->create_text_mesh();
		fractals_text->set_size(text_size);
		fractals_text->transform.position = { x_offset, y_offset, 0.0f };
		fractals_button = std::make_unique<TextButton>(fractals_text.get());
		fractals_button->set_text("Fractals");
		fractals_button->set_index(0);
		fractals_button->set_trigger([&](void) {
			game->push_scene("fractals");
		});
		overlay_meshes.add_instance(std::move(fractals_text));

		y_offset -= 0.1f;
		auto ripples_text = game->create_text_mesh();
		ripples_text->set_size(text_size);
		ripples_text->transform.position = { x_offset, y_offset, 0.0f };
		ripples_button = std::make_unique<TextButton>(ripples_text.get());
		ripples_button->set_text("Ripple Pond");
		ripples_button->set_index(1);
		ripples_button->set_trigger([&](void) {
			game->push_scene("ripplepond");
		});
		overlay_meshes.add_instance(std::move(ripples_text));

		y_offset -= 0.1f;
		auto caustics_text = game->create_text_mesh();
		caustics_text->set_size(text_size);
		caustics_text->transform.position = { x_offset, y_offset, 0.0f };
		caustics_button = std::make_unique<TextButton>(caustics_text.get());
		caustics_button->set_text("Water Caustics");
		caustics_button->set_index(1);
		caustics_button->set_trigger([&](void) {
			game->push_scene("caustics");
		});
		overlay_meshes.add_instance(std::move(caustics_text));

		y_offset -= 0.1f;
		auto quit_text = game->create_text_mesh();
		quit_text->set_size(text_size);
		quit_text->transform.position = { x_offset, y_offset, 0.0f };
		quit_button = std::make_unique<TextButton>(quit_text.get());
		quit_button->set_text("Quit");
		quit_button->set_index(2);
		quit_button->set_trigger([&](void) {
			game->set_done(true);
		});
		overlay_meshes.add_instance(std::move(quit_text));
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
			game->get<UIManager>()->trigger_focus();
			break;
		case SDLK_w:
		case SDLK_UP:
			game->get<UIManager>()->prev_control();
			break;
		case SDLK_s:
		case SDLK_DOWN:
			game->get<UIManager>()->next_control();
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