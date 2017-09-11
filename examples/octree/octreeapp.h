#pragma once

#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <dukat/dukat.h>

#define USE_MULTITHREADING

namespace dukat
{
	class Entity;
	struct Rect;
	class Sprite;
	class Surface;
	struct Texture;

	class OctreeScene : public Scene, public Controller
	{
	private:
		const int texture_width = 800;
		const int texture_height = 600;
		Game2* game;
		// number of render threads.
		int thread_count; 
		// number of chunks to be rendered individually.
		int chunk_count;
		// number of chunks that have been rendered this frame.
		int finished_count;

		bool show_bounding_body;

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

		// Renders a screen segment.
		void render_segment(const Rect& rect);
		// Render thread loop function.
		void thread_render_loop(void);
		// Loads a vox model and sets it as the entity model.
		void load_model(const std::string& model);

	public:
		OctreeScene(Game2* game);
		~OctreeScene(void);

		bool handle_keyboard(const SDL_Event& e);
		void update_debug_text(void);
		void update_texture(void);
		void update(float delta);
		void render(void);

	};
}