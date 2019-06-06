// inputapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "inputapp.h"

namespace dukat
{
	InputScene::InputScene(Game2* game2) : Scene2(game2)
	{
		auto settings = game->get_settings();

		// Set up default camera centered around origin
		auto camera = std::make_unique<Camera2>(game, Vector2(320, 180));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		create_sprites();

		// Set up info text
		auto info_layer = game->get_renderer()->create_layer("overlay", 25.0f);
		info_layer->stage = RenderStage::OVERLAY;
		info_text = game->create_text_mesh();
		info_text->set_size(8.0f);
		info_text->transform.position = Vector3(-0.5f * (float)320, 0.4f * (float)180, 0.0f);
		info_text->transform.update();
		std::stringstream ss;
		ss << "Device: " << game->get_devices()->active->get_name() << std::endl;
		info_text->set_text(ss.str());
		info_layer->add(info_text.get());

		// Set up debug layer
		auto debug_layer = game->get_renderer()->create_layer("debug", 1000.0f);
		debug_layer->stage = RenderStage::OVERLAY;
		debug_text = game->create_text_mesh();
		debug_text->set_size(4.0f);
		debug_text->transform.position = Vector3(-0.5f * (float)640, -0.5f * (float)360, 0.0f);
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

		game->get_devices()->subscribe(this, Events::DeviceBound);

		game->set_controller(this);
	}

	InputScene::~InputScene(void)
	{
		game->get_devices()->unsubscribe(this, Events::DeviceBound);
	}

	void InputScene::create_sprites(void)
	{
		auto layer = game->get_renderer()->create_layer("main", 1.0f);

		// Mask
		mask_sprite = std::make_unique<Sprite>(game->get_textures()->get("controller.png"));
		mask_sprite->z = 100;
		layer->add(mask_sprite.get());

		auto texture = game->get_textures()->get("white.png");
		Rect r{ 0, 0, 16, 16 };
		Color c{ 0xff, 0x0, 0xff, 0x0 };

		// Top buttons
		button_sprites[0] = std::make_unique<Sprite>(texture, r);
		button_sprites[0]->p = Vector2{ -104, -50 };
		button_sprites[0]->color = c;
		layer->add(button_sprites[0].get());
		button_sprites[1] = std::make_unique<Sprite>(texture, r);
		button_sprites[1]->p = Vector2{ 104, -50 };
		button_sprites[1]->color = c;
		layer->add(button_sprites[1].get());

		// Triggers
		ltrigger_sprite = std::make_unique<Sprite>(texture, r);
		ltrigger_sprite->p = Vector2{ -104, -66 };
		ltrigger_sprite->color = c;
		layer->add(ltrigger_sprite.get());
		rtrigger_sprite = std::make_unique<Sprite>(texture, r);
		rtrigger_sprite->p = Vector2{ 104, -66 };
		rtrigger_sprite->color = c;
		layer->add(rtrigger_sprite.get());

		// Left cluster
		dir_sprites[0] = std::make_unique<Sprite>(texture, r);
		dir_sprites[0]->p = Vector2{ -104, 14 };
		dir_sprites[0]->color = c;
		layer->add(dir_sprites[0].get());
		dir_sprites[1] = std::make_unique<Sprite>(texture, r);
		dir_sprites[1]->p = Vector2{ -88, -2 };
		dir_sprites[1]->color = c;
		layer->add(dir_sprites[1].get());
		dir_sprites[2] = std::make_unique<Sprite>(texture, r);
		dir_sprites[2]->p = Vector2{ -120, -2 };
		dir_sprites[2]->color = c;
		layer->add(dir_sprites[2].get());
		dir_sprites[3] = std::make_unique<Sprite>(texture, r);
		dir_sprites[3]->p = Vector2{ -104, -18 };
		dir_sprites[3]->color = c;
		layer->add(dir_sprites[3].get());

		// Right cluster
		button_sprites[2] = std::make_unique<Sprite>(texture, r);
		button_sprites[2]->p = Vector2{ 104, 14 };
		button_sprites[2]->color = c;
		layer->add(button_sprites[2].get());
		button_sprites[3] = std::make_unique<Sprite>(texture, r);
		button_sprites[3]->p = Vector2{ 120, -2 };
		button_sprites[3]->color = c;
		layer->add(button_sprites[3].get());
		button_sprites[4] = std::make_unique<Sprite>(texture, r);
		button_sprites[4]->p = Vector2{ 88, -2 };
		button_sprites[4]->color = c;
		layer->add(button_sprites[4].get());
		button_sprites[5] = std::make_unique<Sprite>(texture, r);
		button_sprites[5]->p = Vector2{ 104, -18 };
		button_sprites[5]->color = c;
		layer->add(button_sprites[5].get());

		// Center buttons
		select_sprite = std::make_unique<Sprite>(texture, r);
		select_sprite->p = Vector2{ -40, -18 };
		select_sprite->color = c;
		layer->add(select_sprite.get());
		start_sprite = std::make_unique<Sprite>(texture, r);
		start_sprite->p = Vector2{ 40, -18 };
		start_sprite->color = c;
		layer->add(start_sprite.get());

		// Thumbsticks
		button_sprites[6] = std::make_unique<Sprite>(texture, r);
		button_sprites[6]->p = Vector2{ -56, 46 };
		button_sprites[6]->color = c;
		layer->add(button_sprites[6].get());
		button_sprites[7] = std::make_unique<Sprite>(texture, r);
		button_sprites[7]->p = Vector2{ 56, 46 };
		button_sprites[7]->color = c;
		layer->add(button_sprites[7].get());

		// Left axis
		left_sprites[0] = std::make_unique<Sprite>(texture, r);
		left_sprites[0]->p = Vector2{ -56, 62 };
		left_sprites[0]->color = c;
		layer->add(left_sprites[0].get());
		left_sprites[1] = std::make_unique<Sprite>(texture, r);
		left_sprites[1]->p = Vector2{ -40, 46 };
		left_sprites[1]->color = c;
		layer->add(left_sprites[1].get());
		left_sprites[2] = std::make_unique<Sprite>(texture, r);
		left_sprites[2]->p = Vector2{ -72, 46 };
		left_sprites[2]->color = c;
		layer->add(left_sprites[2].get());
		left_sprites[3] = std::make_unique<Sprite>(texture, r);
		left_sprites[3]->p = Vector2{ -56, 30 };
		left_sprites[3]->color = c;
		layer->add(left_sprites[3].get());

		// Right axis
		right_sprites[0] = std::make_unique<Sprite>(texture, r);
		right_sprites[0]->p = Vector2{ 56, 62 };
		right_sprites[0]->color = c;
		layer->add(right_sprites[0].get());
		right_sprites[1] = std::make_unique<Sprite>(texture, r);
		right_sprites[1]->p = Vector2{ 72, 46 };
		right_sprites[1]->color = c;
		layer->add(right_sprites[1].get());
		right_sprites[2] = std::make_unique<Sprite>(texture, r);
		right_sprites[2]->p = Vector2{ 40, 46 };
		right_sprites[2]->color = c;
		layer->add(right_sprites[2].get());
		right_sprites[3] = std::make_unique<Sprite>(texture, r);
		right_sprites[3]->p = Vector2{ 56, 30 };
		right_sprites[3]->color = c;
		layer->add(right_sprites[3].get());
	}

	void InputScene::handle_keyboard(const SDL_Event& e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;
		}
	}

	void InputScene::update(float delta)
	{
		Scene2::update(delta);

		auto dev = game->get_devices()->active;

		button_sprites[0]->color.a = dev->is_pressed(InputDevice::Button1) ? 1.0f : 0.0f;
		button_sprites[1]->color.a = dev->is_pressed(InputDevice::Button2) ? 1.0f : 0.0f;
		button_sprites[2]->color.a = dev->is_pressed(InputDevice::Button3) ? 1.0f : 0.0f; 
		button_sprites[3]->color.a = dev->is_pressed(InputDevice::Button4) ? 1.0f : 0.0f;
		button_sprites[4]->color.a = dev->is_pressed(InputDevice::Button5) ? 1.0f : 0.0f;
		button_sprites[5]->color.a = dev->is_pressed(InputDevice::Button6) ? 1.0f : 0.0f;
		button_sprites[6]->color.a = dev->is_pressed(InputDevice::Button7) ? 1.0f : 0.0f;
		button_sprites[7]->color.a = dev->is_pressed(InputDevice::Button8) ? 1.0f : 0.0f;
		select_sprite->color.a = dev->is_pressed(InputDevice::Select) ? 1.0f : 0.0f;
		start_sprite->color.a = dev->is_pressed(InputDevice::Start) ? 1.0f : 0.0f;
		dir_sprites[0]->color.a = dev->is_pressed(InputDevice::Down) ? 1.0f : 0.0f;
		dir_sprites[1]->color.a = dev->is_pressed(InputDevice::Right) ? 1.0f : 0.0f;
		dir_sprites[2]->color.a = dev->is_pressed(InputDevice::Left) ? 1.0f : 0.0f;
		dir_sprites[3]->color.a = dev->is_pressed(InputDevice::Up) ? 1.0f : 0.0f;
		left_sprites[0]->color.a = dev->ly < 0.0f ? std::abs(dev->ly) : 0.0f;
		left_sprites[1]->color.a = dev->lx > 0.0f ? std::abs(dev->lx) : 0.0f;
		left_sprites[2]->color.a = dev->lx < 0.0f ? std::abs(dev->lx) : 0.0f;
		left_sprites[3]->color.a = dev->ly > 0.0f ? std::abs(dev->ly) : 0.0f;
		right_sprites[0]->color.a = dev->ry < 0.0f ? std::abs(dev->ry) : 0.0f;
		right_sprites[1]->color.a = dev->rx > 0.0f ? std::abs(dev->rx) : 0.0f;
		right_sprites[2]->color.a = dev->rx < 0.0f ? std::abs(dev->rx) : 0.0f;
		right_sprites[3]->color.a = dev->ry > 0.0f ? std::abs(dev->ry) : 0.0f;
		ltrigger_sprite->color.a = dev->lt;
		rtrigger_sprite->color.a = dev->rt;
	}

	void InputScene::receive(const Message & msg)
	{
		switch (msg.event)
		{
		case Events::DeviceBound:
		{
			std::stringstream ss;
			ss << "Device: " << game->get_devices()->active->get_name() << std::endl;
			info_text->set_text(ss.str());
			break;
		}
		}
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/input.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game2 app(settings);
		app.add_scene("main", std::make_unique<dukat::InputScene>(&app));
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