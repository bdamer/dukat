// octreeapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "octreeapp.h"
#include "entity.h"
#include "octreebuilder.h"

namespace dukat
{
	OctreeScene::OctreeScene(Game2* game) : game(game), show_bounding_body(false)
	{
		auto layer = game->get_renderer()->create_layer("main", 1.0f);

		auto settings = game->get_settings();

		// Set up default camera centered around origin
		auto camera = std::make_unique<Camera2>(game, Vector2(texture_width, texture_height));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		game->get_renderer()->set_camera(std::move(camera));

		// Set up "fake" camera for raytracer
		ray_camera = std::make_unique<FirstPersonCamera3>(game);
		ray_camera->set_movement_speed(10.0f);
		ray_camera->set_clip(0.01f, 1000.0f);
		ray_camera->set_vertical_fov(55.0f);
		ray_camera->transform.position = Vector3(0.0f, 1.0f, -200.0f);
		ray_camera->transform.dir = Vector3(0.0f, 0.0f, 1.0f);
		ray_camera->transform.up = Vector3(0.0f, 1.0f, 0.0f);
		ray_camera->transform.right = Vector3(-1.0f, 0.0f, 0.0f);
		ray_camera->resize(texture_width, texture_height);
		ray_camera->mouse_look = false;

		// Initialize surface once - ABGR8888 is closest to OpenGL native
		surface = std::make_unique<Surface>(texture_width, texture_height, SDL_PIXELFORMAT_ABGR8888);
		
		// Initialize texture once
		texture = std::make_unique<Texture>(texture_width, texture_height);
		texture->load_data(*surface);
		Rect r = { 0, 0, texture_width, texture_height };
		sprite = std::make_unique<Sprite>(texture.get(), r);
		layer->add(sprite.get());

		// Set up info text
		info_text = game->create_text_mesh();
		info_text->set_size(12.0f);
		info_text->transform.position = Vector3(
			-0.5f * (float)texture_width, 0.40f * (float)texture_height, 0.0f);
		info_text->transform.update();
		std::stringstream ss;
		ss << "Octree Test" << std::endl
			<< "WASD: Move camera position" << std::endl
			<< "TAB: Toggle mouse look" << std::endl
			<< "B: Show bounding sphere" << std::endl
			<< "1,2,3: Load different model" << std::endl;
		info_text->set_text(ss.str());
		layer->add(info_text.get());

		// Set up debug layer
		auto debug_layer = game->get_renderer()->create_layer("debug", 1000.0f);
		debug_text = game->create_text_mesh();
		debug_text->set_size(10.0f);
		debug_text->transform.position = Vector3(-0.5f * (float)texture_width, -0.5f * (float)texture_height, 0.0f);
		debug_text->transform.update();
		debug_layer->add(debug_text.get());
		debug_layer->hide();

		// Create entity 
		entity = std::make_unique<Entity>();
		//OctreeBuilder builder;
		//entity->set_octree(builder.build_sphere(64));
		//entity->set_octree(builder.build_planetoid(48, 8));
		//entity->set_octree(builder.build_cube(32));
		load_model("../assets/models/earth.vox");
		entity->set_bb(std::make_unique<BoundingSphere>(Vector3::origin, 56.0f));

#ifdef USE_MULTITHREADING
		// Create worker threads
		thread_count = settings.get_int("game->get_renderer().threads", 1);
		chunk_count = settings.get_int("game->get_renderer().chunks", 1);
		log->info("Creating {} render threads.", thread_count);
		for (auto i = 0; i < thread_count; i++) {
			thread_pool.push_back(std::thread(&OctreeScene::thread_render_loop, this));
		}
#endif

		game->set_controller(this);
	}

	OctreeScene::~OctreeScene(void)
	{
#ifdef USE_MULTITHREADING
		// wait for worker threads to finish
		std::for_each(thread_pool.begin(), thread_pool.end(),
			std::mem_fn(&std::thread::join));
#endif
	}

	void OctreeScene::load_model(const std::string& file)
	{
		auto is = std::fstream(file, std::fstream::in | std::fstream::binary);
		if (!is)
			throw std::runtime_error("Could not open file");
		VoxModel model;
		is >> model;
 		is.close();
		entity->set_octree(model.get_data());
	}

	void OctreeScene::handle_keyboard(const SDL_Event & e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			game->set_done(true);
			break;
		case SDLK_TAB:
			ray_camera->mouse_look = !ray_camera->mouse_look;
			break;
		case SDLK_1:
			load_model("../assets/models/earth.vox");
			entity->set_bb(std::make_unique<BoundingSphere>(Vector3::origin, 56.0f));
			break;
		case SDLK_2:
			load_model("../assets/models/sphere.vox");
			entity->set_bb(std::make_unique<BoundingSphere>(Vector3::origin, 64.0f));
			break;
		case SDLK_3:
			load_model("../assets/models/cube.vox");
			entity->set_bb(std::make_unique<BoundingSphere>(Vector3::origin, 32.0f));
			break;
		case SDLK_b:
			show_bounding_body = !show_bounding_body;
			break;
		case SDLK_o:
			if (e.key.keysym.mod & KMOD_CTRL)
			{
				log->info("Saving model to ../assets/model.vox");
				VoxModel model;
				model.set_data(entity->get_octree());
				auto os = std::fstream("../assets/model.vox", std::fstream::out | std::fstream::binary);
				if (!os)
					throw std::runtime_error("Could not open file");
				os << model;
				os.close();
				entity->set_octree(model.get_data());
			}
			break;
		}
	}

	void OctreeScene::update_debug_text(void)
	{
		std::stringstream ss;
		auto window = game->get_window();
		auto cam = game->get_renderer()->get_camera();
		ss << "WIN: " << window->get_width() << "x" << window->get_height()
			<< " VIR: " << cam->transform.dimension.x << "x" << cam->transform.dimension.y
			<< " FPS: " << game->get_fps()
			<< " SAMPL: " << dukat::perfc.avg(dukat::PerformanceCounter::SAMPLES)
			<< " BB: " << dukat::perfc.avg(dukat::PerformanceCounter::BB_CHECKS) << std::endl;
		debug_text->set_text(ss.str());
	}
	
	void OctreeScene::update_texture(void)
	{
		GLenum format, type;
		surface->query_pixel_format(format, type);
		glBindTexture(GL_TEXTURE_2D, texture->id);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, surface->get_width(), surface->get_height(),
			format, type, surface->get_surface()->pixels);
#ifdef _DEBUG
		glBindTexture(GL_TEXTURE_2D, 0);
#endif
	}

	void OctreeScene::update(float delta)
	{
		// Rotate object about the y axis
		Quaternion q;
		q.set_to_rotate_y(0.25f * delta);
		entity->transform.rot *= q;

		entity->update(delta);
		ray_camera->update(delta);
		game->get_renderer()->get_camera()->update(delta);
	}

	void OctreeScene::render(void)
	{
#ifdef USE_MULTITHREADING
		// Reset counter and queue render tasks
		finished_count = 0;
		const int segment_height = texture_height / chunk_count;
		for (int i = 0; i < chunk_count; i++)
		{
			std::lock_guard<std::mutex> lk(mtx);
			work_queue.push(dukat::Rect{ 0, i * segment_height, texture_width, (i + 1) * segment_height });
			cond1.notify_one();
		}

		// While main thread is waiting for the workers, present the current screen buffer
		game->get_renderer()->render();

		// Block until all chunks have been rendered
		std::unique_lock<std::mutex> lk(mtx);
		cond2.wait(lk, [&] { return finished_count == chunk_count || game->is_done(); });
#else
		// Single-threaded rendering
		Game2::render();
		render_segment({0,0,texture_width,texture_height});
#endif

		// Render white dot at center of screen and update screen buffer
		surface->set_pixel(texture_width / 2, texture_height / 2, 0xffffffff);
		update_texture();
	}

	void OctreeScene::thread_render_loop(void)
	{
		dukat::Rect rect;
		while (!game->is_done())
		{
			{
				std::unique_lock<std::mutex> lk(mtx);
				cond1.wait(lk, [this] { return !work_queue.empty(); });
				rect = work_queue.front();
				work_queue.pop();
			}

			render_segment(rect);

			std::lock_guard<std::mutex> lk(mtx);
			finished_count++;
			cond2.notify_one();
		}
	}

	void OctreeScene::render_segment(const Rect& rect)
	{
		auto cam = ray_camera.get();
		const auto aspect_ratio = cam->get_aspect_ratio();
		const auto near_z = cam->get_near_clip();
		const auto far_z = cam->get_far_clip();

		// Compute tan of half-angle of field of view
		const auto fov_x = std::tan(deg_to_rad(0.5f * cam->get_horizontal_fov()));
		const auto fov_y = std::tan(deg_to_rad(0.5f * cam->get_vertical_fov()));

		Ray3 ray(cam->transform.position, Vector3::origin);
		auto yf = 0.0f;	// shift on the y for position of current ray [-1,1]
		auto xf = 0.0f;	// shift on the x for position of current ray [-1,1]

		const SDL_Color magenta = { 255, 0, 255, 255 };
		const SDL_Color empty = { 0, 0, 0, 0 };
		const SDL_Color* data;
		for (auto v = rect.y; v < (rect.y + rect.h); v++)
		{
			// Using negative factor to flip y so that -1 is up 
			yf = -fov_y * ((float)(2 * v - texture_height) / (float)texture_height);
			for (auto u = rect.x; u < (rect.x + rect.w); u++)
			{
				xf = fov_x * ((float)(2 * u - texture_width) / (float)texture_width);
				ray.dir.x = cam->transform.dir.x + cam->transform.right.x * xf + cam->transform.up.x * yf;
				ray.dir.y = cam->transform.dir.y + cam->transform.right.y * xf + cam->transform.up.y * yf;
				ray.dir.z = cam->transform.dir.z + cam->transform.right.z * xf + cam->transform.up.z * yf;
				// Not normalizing here because vector is close enough to normal form for our purposes
				//ray.dir.normalize_fast();

				auto best_z = far_z;
				data = &empty;
				//for (vector<Entity>::const_iterator it = entities.begin(); it != entities.end(); ++it)
				for (int i = 0; i < 1; i++)
				{
					auto e = entity.get();
					auto t = e->intersects(ray, near_z, best_z);
					if (t == no_intersection)
						continue;
					if (show_bounding_body)
						data = &magenta;
	
					auto sample = entity->sample(ray, near_z, best_z);
					if (sample == nullptr)
						continue;
					data = sample;
					best_z = t;
				}

				surface->set_pixel(u, v, *data);
			}
		}
	}

}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/octree.ini";
		if (argc > 1)
		{
			config = argv[1];
		}
		dukat::Settings settings(config);
		dukat::Game2 app(settings);
		app.add_scene("main", std::make_unique<dukat::OctreeScene>(&app));
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