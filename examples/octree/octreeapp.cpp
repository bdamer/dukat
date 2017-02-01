// octreeapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "octreeapp.h"
#include "entity.h"
#include "octreebuilder.h"

#include <dukat/aabb3.h>
#include <dukat/application.h>
#include <dukat/boundingsphere.h>
#include <dukat/devicemanager.h>
#include <dukat/inputdevice.h>
#include <dukat/light.h>
#include <dukat/log.h>
#include <dukat/mathutil.h>
#include <dukat/quaternion.h>
#include <dukat/ray3.h>
#include <dukat/rect.h>
#include <dukat/renderlayer2.h>
#include <dukat/settings.h>
#include <dukat/sprite.h>
#include <dukat/surface.h>
#include <dukat/texturecache.h>
#include <dukat/vector3.h>
#include <dukat/voxmodel.h>

namespace dukat
{
	void Game::init(void)
	{
		Game2::init();

		auto layer = renderer->create_layer("main", 1.0f);

		// Set up default camera centered around origin
		auto camera = std::make_unique<Camera2>(window.get(), Vector2(texture_width, texture_height));
		camera->set_clip(settings.get_float("camera.nearclip"), settings.get_float("camera.farclip"));
		camera->refresh();
		renderer->set_camera(std::move(camera));

		// Set up "fake" camera for raytracer
		ray_camera = std::make_unique<FirstPersonCamera3>(window.get(), this);
		ray_camera->set_movement_speed(10.0f);
		ray_camera->set_clip(0.01f, 1000.0f);
		ray_camera->set_fov(55.0f);
		ray_camera->transform.position = Vector3(0.0f, 1.0f, -200.0f);
		ray_camera->transform.dir = Vector3(0.0f, 0.0f, 1.0f);
		ray_camera->transform.up = Vector3(0.0f, 1.0f, 0.0f);
		ray_camera->transform.right = Vector3(-1.0f, 0.0f, 0.0f);
		ray_camera->resize(texture_width, texture_height);
		ray_camera->mouse_look = false;

		// Initialize surface once
		surface = std::make_unique<Surface>(texture_width, texture_height, SDL_PIXELFORMAT_RGBA8888);

		// Initialize texture once
		texture = std::make_unique<Texture>(texture_width, texture_height);
		glGenTextures(1, &texture->id);
		Rect r = { 0, 0, texture_width, texture_height };
		sprite = std::make_unique<Sprite>(texture.get(), r);
		layer->add(sprite.get());

		// Set up info text
		info_text = create_text_mesh(12.0f);
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
		auto debug_layer = renderer->create_layer("debug", 1000.0f);
		debug_text = create_text_mesh(10.0f);
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
		thread_count = settings.get_int("renderer.threads", 1);
		chunk_count = settings.get_int("renderer.chunks", 1);
		logger << "Creating " << thread_count << " render threads." << std::endl;
		for (auto i = 0; i < thread_count; i++) {
			thread_pool.push_back(std::thread(&Game::thread_render_loop, this));
		}
#endif
	}

	void Game::load_model(const std::string& file)
	{
		auto is = std::fstream(file, std::fstream::in | std::fstream::binary);
		if (!is)
			throw std::runtime_error("Could not open file");
		VoxModel model;
		is >> model;
 		is.close();
		entity->set_octree(model.get_data());
	}

	void Game::handle_keyboard(const SDL_Event & e)
	{
		switch (e.key.keysym.sym)
		{
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
				logger << "Saving model to ../assets/model.vox" << std::endl;
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

		default:
			Game2::handle_keyboard(e);
		}
	}

	void Game::update_debug_text(void)
	{
		std::stringstream ss;
		auto cam = renderer->get_camera();
		ss << "WIN: " << window->get_width() << "x" << window->get_height()
			<< " VIR: " << cam->transform.dimension.x << "x" << cam->transform.dimension.y
			<< " FPS: " << get_fps()
			<< " SAMPL: " << dukat::perfc.avg(dukat::PerformanceCounter::SAMPLES)
			<< " BB: " << dukat::perfc.avg(dukat::PerformanceCounter::BB_CHECKS) << std::endl;
		debug_text->set_text(ss.str());
	}
	
	void Game::update_texture(void)
	{
		GLenum format, type;
		surface->query_pixel_format(format, type);

		glBindTexture(GL_TEXTURE_2D, texture->id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, surface->get_width(), surface->get_height(), 0,
			format, type, surface->get_surface()->pixels);
#ifdef _DEBUG
		glBindTexture(GL_TEXTURE_2D, 0);
#endif
	}

	void Game::update(float delta)
	{
		// Rotate object about the y axis
		Quaternion q;
		q.set_to_rotate_y(0.25f * delta);
		entity->transform.rot *= q;

		entity->update(delta);
		ray_camera->update(delta);
		
		Game2::update(delta);
	}

	void Game::render(void)
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
		Game2::render();

		// Block until all chunks have been rendered
		std::unique_lock<std::mutex> lk(mtx);
		cond2.wait(lk, [&] { return finished_count == chunk_count; });
#else
		// Single-threaded rendering
		Game2::render();
		render_segment({0,0,texture_width,texture_height});
#endif

		// Render white dot at center of screen and update screen buffer
		surface->set_pixel(texture_width / 2, texture_height / 2, 0xffffffff);
		update_texture();
	}

	void Game::thread_render_loop(void)
	{
		dukat::Rect rect;
		while (!is_done())
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

	void Game::render_segment(const Rect& rect)
	{
		auto cam = ray_camera.get();
		const auto aspect_ratio = cam->get_aspect_ratio();
		const auto near_z = cam->get_near_clip();
		const auto far_z = cam->get_far_clip();

		// Compute tan of half-angle of field of view
		const auto fov_x = std::tan(deg_to_rad(0.5f * cam->get_fov()));
		const auto fov_y = std::tan(deg_to_rad(0.5f * cam->get_fov()) / aspect_ratio);

		Ray3 ray(cam->transform.position, Vector3::origin);
		auto yf = 0.0f;	// shift on the y for position of current ray [-1,1]
		auto xf = 0.0f;	// shift on the x for position of current ray [-1,1]

		const SDL_Color empty = { 0, 0, 0, 0 };
		const SDL_Color magenta = { 255, 0, 255, 255 };
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
				ray.dir.normalize();

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

	void Game::release(void)
	{
#ifdef USE_MULTITHREADING
		// wait for worker threads to finish
		std::for_each(thread_pool.begin(), thread_pool.end(),
			std::mem_fn(&std::thread::join));
#endif
		Game2::release();
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
		dukat::Game app(settings);
		return app.run();
	}
	catch (const std::exception& e)
	{
		dukat::logger << "Application failed with error." << std::endl << e.what() << std::endl;
		return -1;
	}
	return 0;
}