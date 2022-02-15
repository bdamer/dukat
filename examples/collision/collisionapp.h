#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class GameObject : public Messenger, public Recipient
	{
	public:
		Vector2 dir;
		CollisionManager2::Body* body;

		GameObject(const Vector2& dir, CollisionManager2::Body* body);
		~GameObject(void);

		void handle_dynamic_collision(const CollisionManager2::Body* other_body, const Collision* collision);
		void handle_static_collision(const Collision* collision);

		void receive(const Message& msg);
	};

	class CollisionScene : public Scene2, public Controller
	{
	private:
		static constexpr auto window_width = 1280;
		static constexpr auto window_height = 720;
		static constexpr auto max_speed = 75.0f;
		static constexpr auto wall_size = 16.0f;
		Vector2 screen_dim;

		RenderLayer2* main_layer;
		DebugEffect2* debug_effect;
		std::unique_ptr<Sprite> cursor;
		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;
		std::vector<std::unique_ptr<GameObject>> objects;
		bool animate;
		bool show_grid;

		void remove_object(void);
		void add_object(void);
		void update_objects(float delta);

	public:
		CollisionScene(Game2* game);
		~CollisionScene(void) { }

		void handle_keyboard(const SDL_Event& e);
		void update(float delta);
	};
}