// inputapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "inputapp.h"

namespace dukat
{
	InputScene::InputScene(Game2* game2) : Scene2(game2), feedback(nullptr), anim(nullptr), text_alpha(0.0f)
	{
		auto settings = game->get_settings();

		// Set up default camera centered around origin
		auto camera = std::make_unique<Camera2>(game);
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->set_resize_handler(fixed_camera(320, 180));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		create_sprites();

		auto info_layer = game->get_renderer()->create_direct_layer("overlay", 25.0f);

		// Set up info text
		info_text = game->create_text_mesh();
		info_text->set_size(8.0f);
		info_text->transform.position = Vector3(-0.45f * 320.0f, 0.25f * 180.0f, 0.0f);
		update_info_text();
		info_layer->add(info_text.get());

		// Set up recording text
		action_text = game->create_text_mesh();
		action_text->set_size(16.0f);
		action_text->transform.position = Vector3(0.0f, -0.25f * 180.0f, 0.0f);
		action_text->valign = TextMeshInstance::Top;
		action_text->halign = TextMeshInstance::Center;
		info_layer->add(action_text.get());

		// Set up debug layer
		auto debug_layer = game->get_renderer()->create_direct_layer("debug", 1000.0f);
		debug_text = game->create_text_mesh();
		debug_text->set_size(4.0f);
		debug_text->transform.position = Vector3(-0.5f * 320.0f, -0.5f * 180.0f, 0.0f);
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
			debug_text->update();
		}, true);

		game->get_devices()->subscribe(this, Events::DeviceBound);

		bind_events();

		game->set_controller(this);
	}

	InputScene::~InputScene(void)
	{
		game->get_devices()->unsubscribe(this, Events::DeviceBound);
	}

	void InputScene::create_sprites(void)
	{
		auto layer = game->get_renderer()->create_composite_layer("main", 1.0f);

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

	void InputScene::bind_events(void)
	{
		auto dev = game->get_devices()->active;
		for (auto i = 0; i < InputDevice::_Count; i++)
		{
			const auto btn = static_cast<InputDevice::VirtualButton>(i);
			dev->on_press(btn, [&, btn](void) { log->info("Pressed: {}", game->get_devices()->active->get_button_name(btn)); });
			dev->bind_long_press(btn, [&, btn](void) { log->info("Long Pressed: {}", game->get_devices()->active->get_button_name(btn)); });
		}
	}

	void InputScene::feedback_heartbeat(void)
	{
		if (feedback != nullptr)
			game->get_devices()->cancel_feedback(feedback);

		log->info("Starting heartbeat.");

		std::vector<FeedbackKey> keys = {
			FeedbackKey{ 0.0f, 0.0f, 0.0f },
			FeedbackKey{ 1.0f, 1.0f, 0.0f },
			FeedbackKey{ 1.2f, 0.0f, 0.0f },
			FeedbackKey{ 1.4f, 1.0f, 0.0f },
			FeedbackKey{ 1.6f, 0.0f, 0.0f },
		};
		auto seq = std::make_unique<FeedbackSequence>(10.0f, keys);
		feedback = game->get_devices()->start_feedback(std::move(seq));
	}

	void InputScene::feedback_hi_lo(void)
	{
		if (feedback != nullptr)
			game->get_devices()->cancel_feedback(feedback);

		log->info("Starting hi-lo.");

		std::vector<FeedbackKey> keys = {
			FeedbackKey{ 0.0f, 0.0f, 0.0f },
			FeedbackKey{ 0.5f, 1.0f, 0.0f },
			FeedbackKey{ 1.0f, 0.0f, 1.0f },
			FeedbackKey{ 1.5f, 1.0f, 0.0f },
			FeedbackKey{ 2.0f, 0.0f, 1.0f },
			FeedbackKey{ 2.5f, 0.0f, 0.0f },
		};
		auto seq = std::make_unique<FeedbackSequence>(10.0f, keys);
		feedback = game->get_devices()->start_feedback(std::move(seq));
	}

	void InputScene::update_info_text(void)
	{
		std::stringstream ss;
		ss << "Device: " << game->get_devices()->active->get_name() << std::endl
			<< "1,2 - Test Rumble Effects" << std::endl
			<< "F5 - Toggle recording" << std::endl
			<< "F6 - Toggle replay" << std::endl;
		info_text->set_text(ss.str());
		info_text->update();
	}

	void InputScene::toggle_record(void)
	{
		auto devs = game->get_devices();

		if (devs->is_replaying())
			devs->stop_replay();

		if (devs->is_recording())
			devs->stop_record();
		else
			devs->start_record("recording");

		if (anim != nullptr)
		{
			game->get_animations()->cancel(anim);
			anim = nullptr;
		}

		if (devs->is_recording())
		{
			action_text->set_color(Color{ 1.0f, 0.0f, 0.0f, 1.0f });
			action_text->set_text("RECORDING");
			action_text->update();
			start_animation();
		}
		else
		{
			action_text->set_alpha(0.0f);
		}
	}

	void InputScene::toggle_replay(void)
	{
		auto devs = game->get_devices();

		if (devs->is_recording())
			devs->stop_record();

		if (devs->is_replaying())
			devs->stop_replay();
		else
			devs->start_replay("recording");

		if (anim != nullptr)
		{
			game->get_animations()->cancel(anim);
			anim = nullptr;
		}

		if (devs->is_replaying())
		{
			action_text->set_color(Color{ 1.0f, 0.0f, 0.0f, 1.0f });
			action_text->set_text("REPLAY");
			action_text->update();
			start_animation();
		}
		else
		{
			action_text->set_alpha(0.0f);
		}
	}

	void InputScene::start_animation(void)
	{
		auto animation = std::make_unique<ValueAnimation<float>>(&text_alpha);
		animation->set_loop(true);
		animation->add_key(AnimationKey<float>(0.25f, 1.0f));
		animation->add_key(AnimationKey<float>(0.75f, 0.0f));
		anim = game->get_animations()->add(std::move(animation));
	}

	void InputScene::handle_keyboard(const SDL_Event& e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;

		case SDLK_1:
			feedback_heartbeat();
			break;
		case SDLK_2:
			feedback_hi_lo();
			break;

		case SDLK_F5:
			toggle_record();
			break;
		case SDLK_F6:
			toggle_replay();
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

		action_text->set_alpha(text_alpha);
	}

	void InputScene::receive(const Message & msg)
	{
		switch (msg.event)
		{
		case Events::DeviceBound:
			update_info_text();
			break;
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
}