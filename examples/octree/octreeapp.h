#pragma once

#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <dukat/game2.h>
#include <dukat/firstpersoncamera3.h>

#define USE_MULTITHREADING

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
		// number of render threads.
		int thread_count; 
		// number of chunks to be rendered individually.
		int chunk_count;
		// number of chunks that have been rendered this frame.
		int finished_count;

		// Multi-threaded rendering
		std::vector<std::thread> thread_pool;
		std::mutex mtx;
		std::condition_variable cond1, cond2;
		std::queue<dukat::Rect> work_queue;

		// Render objects
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
		void release(void);

		// Renders a screen segment.
		void render_segment(const Rect& rect);
		// Render thread loop function.
		void thread_render_loop(void);
		// Loads a vox model and sets it as the entity model.
		void load_model(const std::string& model);

	public:
		Game(const Settings& settings) : Game2(settings) { }
		~Game(void) { }
	};
}