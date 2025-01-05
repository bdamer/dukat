// text.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "text.h"

namespace dukat
{
	static const auto sample_text1 = 
		"I sing the <#dd0907>body electric</>,\n"
		"The <#fbf305>armies</> of those I <#f20884>love</> engirth me and I engirth them,\n"
		"They will not let me off till I go with them, <#02abea>respond</> to them,\n"
		"And <#ff6403>discorrupt</> them, and charge them full with the <#0000d3>charge</> of the soul.\n\n"
		"Was it <#4700a5>doubted</> that those who corrupt their own bodies <#1fb714>conceal</> themselves ?\n"
		"And if those who defile the <#90713a>living</> are as bad as they who defile the <#562c05>dead</> ?\n"
		"And if the body does not do <#c0c0c0>fully</> as much as the <#808080>soul</> ?\n"
		"And if the body were not the <#006412>soul</>, what is the <#404040>soul</> ?";
	static const auto sample_text2 =
		"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras interdum fringilla erat, et consectetur urna ultricies vel. "
		"Nulla vitae euismod lacus. Mauris id leo id sem congue euismod at non tellus. Donec luctus, nunc ac scelerisque eleifend, "
		"eros ante venenatis lectus, nec feugiat nunc lectus vitae lacus. Sed condimentum id augue vel posuere. Pellentesque sodales "
		"risus malesuada metus lobortis porta. Pellentesque a congue nisl. Pellentesque habitant morbi tristique senectus et netus et "
		"malesuada fames ac turpis egestas. Mauris eleifend pellentesque enim, nec euismod erat aliquet eu. Etiam eget condimentum "
		"libero, at posuere nunc. Duis sit amet nulla sed nulla iaculis molestie.";

	TextScene::TextScene(Game2* game2) : Scene2(game2), font_size(32)
	{
		auto settings = game->get_settings();
		auto renderer = game->get_renderer();
		renderer->set_clear_color(Color{ 0.3f, 0.3f, 0.3f, 0.0f });
		auto layer = renderer->create_direct_layer(layer_name, 1.0f);

		// Set up default camera centered around origin
		auto window = game->get_window();
		auto camera = std::make_unique<Camera2>(game);
		camera->set_clip(settings.get_float(settings::video_camera_nearclip), settings.get_float(settings::video_camera_farclip));
		camera->set_resize_handler(fixed_height_camera(window->get_height()));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		// Set up center text
		center_text = game->create_text_mesh();
		center_text->halign = TextMeshInstance::Center;
		center_text->valign = TextMeshInstance::Center;
		center_text->set_size(static_cast<float>(font_size));

		reset_text_scroll();
		center_text->update(0.0f);
		layer->add(center_text.get());

		// Set up info text
		info_text = game->create_text_mesh();
		info_text->set_size(24.0f);
		info_text->transform.position = Vector3(
			-0.475f * static_cast<float>(window->get_width()),
			0.2f * static_cast<float>(window->get_height()),
			0.0f);
		const auto info_str =
			"<F1>Scrolling text\n"
			"<F2>Line Breaks\n"
			"<0-5> Change font\n"
			"<-+> Change font size\n"
			"<LEFT,RIGHT> Change horizontal spacing\n"
			"<DOWN,UP> Change line height\n"
			"<F11> Toggle info";
		info_text->set_text(info_str);
		info_text->update(0.0f);
		layer->add(info_text.get());

		// Set up debug layer
		auto debug_layer = game->get_renderer()->create_direct_layer("debug", 1000.0f);
		debug_text = game->create_text_mesh();
		debug_text->set_size(12.0f);
		debug_text->transform.position = Vector3(
			-0.5f * static_cast<float>(window->get_width()), 
			-0.5f * static_cast<float>(window->get_height()), 
			0.0f);
		debug_text->transform.update();
		debug_layer->add(debug_text.get());
		debug_layer->hide();

		game->get<TimerManager>()->create(1.0f, [&]() {
			std::stringstream ss;
			auto window = game->get_window();
			auto cam = game->get_renderer()->get_camera();
			ss << "WIN: " << window->get_width() << "x" << window->get_height()
				<< " VIR: " << cam->transform.dimension.x << "x" << cam->transform.dimension.y
				<< " FPS: " << game->get_fps()
				<< " MESH: " << dukat::perfc.avg(dukat::PerformanceCounter::MESHES)
				<< " VERT: " << dukat::perfc.avg(dukat::PerformanceCounter::VERTICES) << std::endl;
			debug_text->set_text(ss.str());
		}, true);

		game->set_controller(this);
	}

	void TextScene::reset_text_scroll(void)
	{
		center_text->set_max_line_width(0.0f);
		center_text->set_text_scroll(sample_text1, 0.05f, std::bind(&TextScene::reset_text_scroll, this));
	}

	void TextScene::reset_line_breaks(void)
	{
		center_text->cancel_scroll();
		center_text->set_max_line_width(24.0f);
		center_text->set_text(sample_text2);
	}

	void TextScene::update(float delta)
	{
		center_text->update(delta);
	}
	
	void TextScene::handle_keyboard(const SDL_Event& e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;
		case SDLK_F1:
			reset_text_scroll();
			break;
		case SDLK_F2:
			reset_line_breaks();
			break;

		case SDLK_F11:
			info_text->visible = !info_text->visible;
			break;

		case SDLK_0:
			center_text->set_font(game->get_fonts()->get("generic.fnt"));
			break;
		case SDLK_1:
			center_text->set_font(game->get_fonts()->get("alagard.fnt"));
			break;
		case SDLK_2:
			center_text->set_font(game->get_fonts()->get("arcadealternate.fnt"));
			break;
		case SDLK_3:
			center_text->set_font(game->get_fonts()->get("blocktopia.fnt"));
			break;
		case SDLK_4:
			center_text->set_font(game->get_fonts()->get("chronotype.fnt"));
			break;
		case SDLK_5:
			center_text->set_font(game->get_fonts()->get("pixeloperator.fnt"));
			break;

		case SDLK_LEFT:
			center_text->set_char_width(0.9f * center_text->get_char_width());
			break;
		case SDLK_RIGHT:
			center_text->set_char_width(1.1f * center_text->get_char_width());
			break;

		case SDLK_UP:
			center_text->set_line_height(0.9f * center_text->get_line_height());
			break;
		case SDLK_DOWN:
			center_text->set_line_height(1.1f * center_text->get_line_height());
			break;

		case SDLK_PLUS:
		case SDLK_EQUALS:
			if (font_size < 32)
			{
				font_size++;
				center_text->set_size(static_cast<float>(font_size));
			}
			break;

		case SDLK_MINUS:
			if (font_size > 1)
			{
				font_size--;
				center_text->set_size(static_cast<float>(font_size));
			}
			break;
		}
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/text.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game2 app(settings);
		app.add_scene("main", std::make_unique<dukat::TextScene>(&app));
		app.push_scene("main");
		return app.run();
	}
	catch (const std::exception& e)
	{
		dukat::log->error("Application failed with error: {}", e.what());
		return -1;
	}
}