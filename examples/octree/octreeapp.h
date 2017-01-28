#pragma once

#include <memory>
#include <dukat/game2.h>
#include <dukat/firstpersoncamera3.h>

namespace dukat
{
	class Entity;
	struct Rect;
	class Sprite;
	class Surface;
	struct Texture;

	class Game : public Game2
	{
	private:
		const int texture_width = 800;
		const int texture_height = 600;

		std::unique_ptr<FirstPersonCamera3> ray_camera;
		std::unique_ptr<Texture> texture;
		std::unique_ptr<Surface> surface;
		std::unique_ptr<Sprite> sprite;
		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;

		std::unique_ptr<Entity> entity;

		void init(void);
		void handle_keyboard(const SDL_Event& e);
		void update_debug_text(void);
		void update_texture(void);
		void update(float delta);
		void render(void);

		void render_segment(const Rect& rect);

		void load_model(const std::string& model);

	public:
		Game(const Settings& settings) : Game2(settings) { }
		~Game(void) { }
	};
}