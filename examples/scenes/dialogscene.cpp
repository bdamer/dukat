#include "stdafx.h"
#include "dialogscene.h"

namespace dukat
{
	DialogScene::DialogScene(Game3* game3) : game(game3), display_mode_idx(-1)
	{
		overlay_meshes.stage = RenderStage::Overlay;
		overlay_meshes.visible = true;

		auto control_idx = 0;

		auto text_mesh = game->create_text_mesh();
		text_mesh->set_text("<#dd0907>Options screen</>");
		title_text = text_mesh.get();
		overlay_meshes.add_instance(std::move(text_mesh));

		text_mesh = game->create_text_mesh();
		mode_button = std::make_unique<TextButton>(text_mesh.get());
		mode_button->set_text(" ");
		mode_button->set_index(control_idx++);
		mode_button->set_cycle([&](int dir) {
			auto new_idx = static_cast<int>(window_mode) + dir;
			if (new_idx < 0)
				new_idx = 2;
			else if (new_idx > 2)
				new_idx = 0;
			window_mode = static_cast<Window::Mode>(new_idx);
			update_mode_button();
		});
		mode_text = text_mesh.get();
		overlay_meshes.add_instance(std::move(text_mesh));

		text_mesh = game->create_text_mesh();
		resolution_button = std::make_unique<TextButton>(text_mesh.get());
		resolution_button->set_text(" ");
		resolution_button->set_index(control_idx++);
		resolution_button->set_cycle([&](int dir) {
			if (display_mode_idx < 0) return; // unsupported
			const auto num_modes = static_cast<int>(display_modes.size());
			display_mode_idx += dir;
			if (display_mode_idx < 0) display_mode_idx += num_modes;
			else if (display_mode_idx >= num_modes)
				display_mode_idx = 0;
			update_resolution_button();
		});
		resolution_text = text_mesh.get();
		overlay_meshes.add_instance(std::move(text_mesh));

		text_mesh = game->create_text_mesh();
		apply_button = std::make_unique<TextButton>(text_mesh.get());
		apply_button->set_text(option_apply);
		apply_button->set_index(control_idx++);
		apply_button->set_trigger([&](void) {
			const auto& mode = display_modes[display_mode_idx];
			game->get_window()->change_mode(window_mode, mode);
			update_display_modes();
		});
		apply_text = text_mesh.get();
		overlay_meshes.add_instance(std::move(text_mesh));

		text_mesh = game->create_text_mesh();
		return_button = std::make_unique<TextButton>(text_mesh.get());
		return_button->set_text(option_back);
		return_button->set_index(control_idx++);
		return_button->set_trigger([&](void) {
			game->pop_scene();
		});
		return_text = text_mesh.get();
		overlay_meshes.add_instance(std::move(text_mesh));
	}

	void DialogScene::update_layout(void)
	{
		const auto x_offset = -1.0f;
		const auto font_scale = 1.0f / 12.0f;
		auto y_offset = 0.25f;

		title_text->set_size(2.f * font_scale);
		title_text->transform.position = { -0.75f, y_offset, 0.0f };

		y_offset -= 0.25f;
		mode_text->set_size(font_scale);
		mode_text->transform.position = { x_offset, y_offset, 0.0f };

		y_offset -= 0.1f;
		resolution_text->set_size(font_scale);
		resolution_text->transform.position = { x_offset, y_offset, 0.0f };

		y_offset -= 0.1f;
		apply_text->set_size(font_scale);
		apply_text->transform.position = { x_offset, y_offset, 0.0f };

		y_offset -= 0.1f;
		return_text->set_size(font_scale);
		return_text->transform.position = { x_offset, y_offset, 0.0f };
	}

	void DialogScene::update_display_modes(void)
	{
		display_modes = list_display_modes(0);
		if (game->get_window()->is_fullscreen())
		{
			display_mode_idx = find_current_display_mode(0, display_modes);
		}
		else
		{
			display_mode_idx = -1;
			const auto width = game->get_window()->get_width();
			const auto height = game->get_window()->get_height();
			for (auto i = 0; i < static_cast<int>(display_modes.size()); i++)
			{
				if (display_modes[i].w == width && display_modes[i].h == height)
				{
					display_mode_idx = i;
					break;
				}
			}
		}
	}

	void DialogScene::update_mode_button(void)
	{
		std::string label(option_mode);
		switch (window_mode)
		{
		case Window::Windowed:
			label += option_windowed;
			break;
		case Window::Borderless:
			label += option_borderless;
			break;
		case Window::Fullscreen:
			label += option_fullscreen;
			break;
		}
		mode_button->set_text(label);
	}

	void DialogScene::update_resolution_button(void)
	{
		if (display_mode_idx >= 0)
			resolution_button->set_text(option_resolution + format_display_mode(display_modes[display_mode_idx]));
		else
			resolution_button->set_text(option_resolution + std::string("Unknown"));
	}

	void DialogScene::activate(void)
	{ 
		auto renderer = game->get_renderer();
		renderer->set_clear_color(Color{ 0.5f, 0.7f, 0.7f, 1.0f });

		auto settings = game->get_settings();
		auto camera = std::make_unique<FixedCamera3>(game, Vector3{ 0.0f, 0.0f, 2.5f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3::unit_y);
		camera->set_vertical_fov(settings.get_float("camera.fov"));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		renderer->set_camera(std::move(camera));

		// Update display mode since this option may have changed externally
		if (game->get_window()->is_fullscreen())
			window_mode = Window::Fullscreen;
		else
			window_mode = game->get_window()->is_borderless() ? Window::Borderless : Window::Windowed;
		update_mode_button();
		update_display_modes();
		update_resolution_button();
		update_layout();

		game->set_controller(this);

		game->get<UIManager>()->add_control(mode_button.get());
		game->get<UIManager>()->add_control(resolution_button.get());
		game->get<UIManager>()->add_control(apply_button.get());
		game->get<UIManager>()->add_control(return_button.get());

		game->get<UIManager>()->first_control();
	}

	void DialogScene::deactivate(void)
	{
		game->get<UIManager>()->remove_control(mode_button.get());
		game->get<UIManager>()->remove_control(resolution_button.get());
		game->get<UIManager>()->remove_control(apply_button.get());
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
		case SDLK_w:
			game->get<UIManager>()->prev_control();
			break;
		case SDLK_DOWN:
		case SDLK_s:
			game->get<UIManager>()->next_control();
			break;
		case SDLK_LEFT:
		case SDLK_a:
			game->get<UIManager>()->cycle_focus(-1);
			break;
		case SDLK_RIGHT:
		case SDLK_d:
			game->get<UIManager>()->cycle_focus(1);
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