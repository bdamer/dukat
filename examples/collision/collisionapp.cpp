// collisionapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "collisionapp.h"

namespace dukat
{
	CollisionScene::CollisionScene(Game2* game2) : Scene2(game2), animate(true), show_grid(true)
	{
		auto cm = game->add_manager<CollisionManager2>();
		cm->set_world_size(2000.0f);
		cm->set_world_depth(4);

		auto settings = game->get_settings();
		// Set up default camera centered around origin
		auto camera = std::make_unique<Camera2>(game);
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->set_resize_handler(fixed_camera(window_width, window_height));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		screen_dim = Vector2{ 0.5f * window_width, 0.5f * window_height };

		main_layer = game->get_renderer()->create_direct_layer("main", 20.0f);
		debug_effect = static_cast<DebugEffect2*>(main_layer->add(std::make_unique<DebugEffect2>(game, 1.0f)));

		// Add cursor
		auto texture = game->get_textures()->get("cursor32.png");
		cursor = std::make_unique<Sprite>(texture);
		cursor->flags = Sprite::align_left | Sprite::align_top;
		main_layer->add(cursor.get());

		// Add walls:
		auto wall = cm->create_body(false); // north
		wall->bb = AABB2{ -screen_dim, Vector2{ screen_dim.x, -screen_dim.y + wall_size } };
		wall = cm->create_body(false); // east
		wall->bb = AABB2{ Vector2{ screen_dim.x - wall_size, -screen_dim.y + wall_size }, Vector2{ screen_dim.x, screen_dim.y - wall_size } };
		wall = cm->create_body(false); // south
		wall->bb = AABB2{ Vector2{ -screen_dim.x, screen_dim.y - wall_size }, Vector2{ screen_dim.x, screen_dim.y } };
		wall = cm->create_body(false); // west
		wall->bb = AABB2{ Vector2{ -screen_dim.x, -screen_dim.y + wall_size }, Vector2{ -screen_dim.x + wall_size, screen_dim.y - wall_size } };

		// Add some objects
		for (auto i = 0; i < 50; i++)
			add_object();

		game->get_devices()->active->on_press(InputDevice::Button1, [&](void) {
			auto ctrl = game->get_devices()->active;
			auto pos = Vector2{ ctrl->rxa, ctrl->rya } - screen_dim;
			auto cm = game->get<CollisionManager2>();
			auto res = cm->find(pos);
			for (auto b : res)
			{
				log->info("Click on {}", b->id);
			}
		});

		// Set up info text
		info_text = game->create_text_mesh();
		info_text->set_size(32.0f);
		info_text->transform.position = Vector3(-0.5f * (float)window_width, 0.3f * (float)window_height, 0.0f);
		main_layer->add(info_text.get());

		game->get<TimerManager>()->create(0.25f, [&]() {
			auto cm = game->get<CollisionManager2>();
			std::stringstream ss;
			ss << "Bodies: " << perfc.avg(PerformanceCounter::BODIES) << std::endl
				<< "Collisions: " << cm->contact_count() << std::endl
				<< "Tests: " << perfc.avg(PerformanceCounter::BB_CHECKS) << std::endl
				<< "<Space> Pause movement" << std::endl
				<< "<g> Toggle grid" << std::endl
				<< "<-,+> Remove / Add object" << std::endl;
			info_text->set_text(ss.str());
			info_text->update();
		}, true);

		// Set up debug layer
		auto debug_layer = game->get_renderer()->create_composite_layer("debug", 1000.0f);
		debug_text = game->create_text_mesh();
		debug_text->set_size(16.0f);
		debug_text->transform.position = Vector3(-0.5f * (float)window_width, -0.5f * (float)window_height, 0.0f);
		debug_layer->add(debug_text.get());
		debug_layer->hide();

		game->get<TimerManager>()->create(1.0f, [&]() {
			std::stringstream ss;
			auto window = game->get_window();
			auto cam = game->get_renderer()->get_camera();
			ss << "WIN: " << window->get_width() << "x" << window->get_height()
				<< " VIR: " << cam->transform.dimension.x << "x" << cam->transform.dimension.y
				<< " FPS: " << game->get_fps()
				<< " MESH: " << perfc.avg(dukat::PerformanceCounter::MESHES)
				<< " VERT: " << perfc.avg(dukat::PerformanceCounter::VERTICES)
				<< " BB: " << perfc.avg(PerformanceCounter::BB_CHECKS)
				<< std::endl;
			debug_text->set_text(ss.str());
			debug_text->update();
		}, true);

		game->set_controller(this);
	}

	void CollisionScene::handle_keyboard(const SDL_Event & e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;
		case SDLK_MINUS:
			remove_object();
			break;
		case SDLK_EQUALS:
			add_object();
			break;
		case SDLK_SPACE:
			animate = !animate;
			break;
		case SDLK_COMMA:
			update_objects(-1.0f / 60.0f);
			break;
		case SDLK_PERIOD:
			update_objects(1.0f / 60.0f);
			break;

		case SDLK_g:
			show_grid = !show_grid;
			if (show_grid)
			{
				debug_effect->set_flags(static_cast<DebugEffect2::Flags>(DebugEffect2::Flags::BODIES | DebugEffect2::Flags::GRID));
			}
			else
			{
				debug_effect->set_flags(DebugEffect2::Flags::BODIES);
			}
			break;
		}
	}

	void CollisionScene::remove_object(void)
	{
		if (!objects.empty())
		{
			game->get<CollisionManager2>()->destroy_body(objects.back()->body);
			objects.pop_back();
		}
	}

	void CollisionScene::add_object(void)
	{
		auto dir = Vector2{ random(-max_speed, max_speed), random(-max_speed, max_speed) };
		auto size = random(10, 20);
		auto seed_pos = screen_dim - Vector2{ wall_size + 0.5f * size, wall_size + 0.5f * size };
		auto pos = random(-seed_pos, seed_pos);
		auto body = game->get<CollisionManager2>()->create_body();
		body->bb = AABB2{ pos - Vector2{ size, size }, pos + Vector2{ size, size } };
		body->mass = static_cast<float>(size * size);
		objects.push_back(std::make_unique<GameObject>(dir, body));
	}

	void CollisionScene::update_objects(float delta)
	{
		for (auto& o : objects)
		{
			o->body->bb += o->dir * delta;
		}
	}

	void CollisionScene::update(float delta)
	{
		auto dev = game->get_devices()->active;
		cursor->p.x = dev->rxa - screen_dim.x;
		cursor->p.y = dev->rya - screen_dim.y;

		if (animate)
			update_objects(delta);

		Scene2::update(delta);
	}

	GameObject::GameObject(const Vector2& dir, CollisionManager2::Body* body) : dir(dir), body(body)
	{
		body->owner = this;
		this->subscribe(this, Events::CollisionBegin);
	}

	GameObject::~GameObject(void)
	{
		this->unsubscribe(this, Events::CollisionBegin);
	}

	void GameObject::handle_dynamic_collision(const CollisionManager2::Body* other_body, const Collision* collision)
	{
		// only process one side of the collision
		if (body->id > other_body->id)
			return;

		// Performs perfectly elastic collision
		auto other_obj = static_cast<GameObject*>(other_body->owner);
		const auto m1 = body->mass;
		const auto m2 = other_body->mass;
		const auto v1 = dir;
		const auto v2 = other_obj->dir;
		dir = v1 * (m1 - m2) / (m1 + m2) + v2 * (2.f * m2) / (m1 + m2);
		other_obj->dir = v2 * (m2 - m1) / (m1 + m2) + v1 * (2.f * m1) / (m1 + m2);
	}

	void GameObject::handle_static_collision(const Collision* collision)
	{
		auto nx = std::abs(collision->normal.x);
		auto ny = std::abs(collision->normal.y);
		if (nx > ny)
			dir.x = -dir.x;
		else
			dir.y = -dir.y;
	}

	void GameObject::receive(const Message& msg)
	{
		switch (msg.event)
		{
		case Events::CollisionBegin:
		{
			if (!body->dynamic)
				return; // nothing to do
			auto other_body = static_cast<const CollisionManager2::Body*>(msg.param1);
			auto collision = static_cast<const Collision*>(msg.param2);
			if (!other_body->dynamic)
				handle_static_collision(collision);
			else
				handle_dynamic_collision(other_body, collision);
			break;
		}
		}
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/collision.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game2 app(settings);
		app.add_scene("main", std::make_unique<dukat::CollisionScene>(&app));
		app.push_scene("main");
		return app.run();
	}
	catch (const std::exception& e)
	{
		dukat::log->error("Application failed with error: {}", e.what());
		return -1;
	}
}