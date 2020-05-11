// text.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "text.h"

namespace dukat
{
	TextScene::TextScene(Game2* game2) : Scene2(game2)
	{
		auto layer = game->get_renderer()->create_direct_layer("main", 1.0f);

		auto settings = game->get_settings();

		// Set up default camera centered around origin
		auto window = game->get_window();
		auto camera = std::make_unique<Camera2>(game, Vector2((float)window->get_width(), (float)window->get_height()));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		// Set up info text
		info_text = game->create_text_mesh();
		info_text->halign = TextMeshInstance::Center;
		info_text->valign = TextMeshInstance::Center;
		info_text->set_size(10.0f);
		std::stringstream ss;
		ss << "I sing the <#dd0907>body electric</>," << std::endl
			<< "The <#fbf305>armies</> of those I <#f20884>love</> engirth me and I engirth them," << std::endl
			<< "They will not let me off till I go with them, <#02abea>respond</> to them," << std::endl
			<< "And <#ff6403>discorrupt</> them, and charge them full with the <#0000d3>charge</> of the soul." << std::endl << std::endl
			<< "Was it <#4700a5>doubted</> that those who corrupt their own bodies <#1fb714>conceal</> themselves ?" << std::endl
			<< "And if those who defile the <#90713a>living</> are as bad as they who defile the <#562c05>dead</> ?" << std::endl
			<< "And if the body does not do <#c0c0c0>fully</> as much as the <#808080>soul</> ?" << std::endl
			<< "And if the body were not the <#006412>soul</>, what is the <#404040>soul</> ?" << std::endl;
		info_text->set_text(ss.str());
		info_text->update(0.0f);
		layer->add(info_text.get());

		// Set up debug layer
		auto debug_layer = game->get_renderer()->create_direct_layer("debug", 1000.0f);
		debug_text = game->create_text_mesh();
		debug_text->set_size(12.0f);
		debug_text->transform.position = Vector3(-0.5f * (float)window->get_width(), -0.5f * (float)window->get_height(), 0.0f);
		debug_text->transform.update();
		debug_layer->add(debug_text.get());
		debug_layer->hide();

		game->get<TimerManager>()->create_timer(1.0f, [&]() {
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
	return 0;
}