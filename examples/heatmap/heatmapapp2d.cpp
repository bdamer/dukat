// heatmapapp2d.cpp : First draft of the heatmap process in 2D - kept just for 
// reference, not included in build.
//

#include "stdafx.h"
#include "heatmapapp.h"

#include <array>

#include <dukat/application.h>
#include <dukat/devicemanager.h>
#include <dukat/inputdevice.h>
#include <dukat/light.h>
#include <dukat/log.h>
#include <dukat/mathutil.h>
#include <dukat/rect.h>
#include <dukat/renderlayer2.h>
#include <dukat/settings.h>
#include <dukat/sprite.h>
#include <dukat/surface.h>
#include <dukat/texturecache.h>

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

		// Initialize texture once
		surface = std::make_unique<Surface>(texture_width, texture_height, SDL_PIXELFORMAT_RGBA8888);
		texture = std::make_unique<Texture>(texture_width, texture_height);
		Rect r = { 0, 0, texture_width, texture_height };
		sprite = std::make_unique<Sprite>(texture.get(), r);
		layer->add(sprite.get());

		// Set up info text
		info_text = create_text_mesh(8.0f);
		info_text->transform.position = Vector3(-0.5f * (float)texture_width, 0.0f, 0.0f);
		info_text->transform.update();
		std::stringstream ss;
		ss << "Heat Test" << std::endl
			<< "Press 1: Load test image" << std::endl
			<< "Press 2: B/W test image" << std::endl
			<< "Press 3: Color test image" << std::endl
			<< std::endl;
		info_text->set_text(ss.str());
		//layer->add(info_text.get());

		// Set up debug layer
		auto debug_layer = renderer->create_layer("debug", 1000.0f);
		debug_text = create_text_mesh(2.0f);
		debug_text->transform.position = Vector3(-0.5f * (float)texture_width, -0.5f * (float)texture_height, 0.0f);
		debug_text->transform.update();
		debug_layer->add(debug_text.get());
		debug_layer->hide();

#ifdef TOPDOWN
		cells.resize(texture_width * texture_height);
#else
		cells.resize(texture_width);
#endif

		reset();
	}

	void Game::handle_keyboard(const SDL_Event & e)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_r:
			reset();
			break;
		default:
			Game2::handle_keyboard(e);
		}
	}

	void Game::reset(void)
	{
		// chance that cells is an emitter (and thus active)
#ifdef TOPDOWN
		const auto chance = 400;
#else
		const auto chance = 20;
#endif

		for (auto i = 0u; i < cells.size(); i++)
		{
			auto is_emitter = (rand() % chance == 0);
			cells[i].active = cells[i].emitter = is_emitter;
			cells[i].delta = 0.0f;
			cells[i].z = 0.0f;
			cells[i].temp = is_emitter ? randf(0.0f, 1.0f) : 0.0f;
			cells[i].veg = 0.0f;
		}

		surface->fill(surface->color(0, 0, 255, 255));
	}

	void Game::render(void)
	{
		Game2::render();

		render_cells();

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
		Game2::update(delta);

		delta *= 8.0f;
		
		// Sink cell used as border on either side of the cells
		Cell sink;
		// min heat threshold - below this no heat transmission takes place
		const auto threshold = 0.05f;
		// max heat capacity of emitter
		const auto emitter_capacity = 5.0f;
		// heat production from each emitter
		const auto emission_rate = 10.0f;
		// capacity of regular cells
		const auto capacity = 0.75f;
		// max percentage of heat lost during transfer
		const auto transfer_loss = 0.1f;
		// temperature loss due to dissipation
		const auto dissipate_loss = 0.1f;
		// elevation increase step size
		const auto elevation_step = 1.0f / (float)texture_height;
		// vegetation growth rate
		const auto growth_rate = 0.15f;

		for (auto i = 0u; i < cells.size(); i++)
		{
			auto& cell = cells[i];
			if (cell.emitter)
			{
				// small chance that emitter status flips...
				if (rand() % 1000 == 0)
				{
					cell.active = !cell.active;
				}
				if (cell.active)
				{
					cell.temp += emission_rate * delta;
				}
			}

			if (cell.temp > threshold)
			{
				// Reset vegetation if temperature is above threshold
				cell.veg = 0.0f;

#ifdef TOPDOWN
				// collect neighboring cells (clockwise, starting at 12')
				int x = i % texture_width;
				int y = i / texture_width;
				std::array<Cell*, 8> neighbors;
				std::array<float, 8> slopes;
				neighbors[0] = (y > 0) ? &cells[i - texture_width] : &cells[(texture_height - 1) * texture_width + x];
				neighbors[1] = (x < texture_width - 1 && y > 0) ? &cells[i - texture_width + 1] : &sink;
				neighbors[2] = (x < texture_width - 1) ? &cells[i + 1] : &cells[y * texture_width];
				neighbors[3] = (x < texture_width - 1 && y < texture_height - 1) ? &cells[i + texture_width + 1] : &sink;
				neighbors[4] = (y < texture_height - 1) ? &cells[i + texture_width] : &cells[x];
				neighbors[5] = (x > 0 && y < texture_height - 1) ? &cells[i + texture_width - 1] : &sink;
				neighbors[6] = (x > 0) ? &cells[i - 1] : &cells[(y + 1) * texture_width - 1];
				neighbors[7] = (x > 0 && y > 0) ? &cells[i - texture_width - 1] : &sink;
#else
				// collect left and right cells
				std::array<Cell*, 2> neighbors;
				std::array<float, 2> slopes;
				neighbors[0] = (i > 0) ? &cells[i - 1] : &cells[texture_width - 1];
				neighbors[1] = (i < texture_width - 1) ? &cells[i + 1] : &cells[0];
#endif

				// compute slopes to each neighboring cell
				float slope_sum = 0.0f;
				for (auto j = 0u; j < slopes.size(); j++)
				{
					auto slope = cell.z - neighbors[j]->z;
					// transfer can only happen on a downslope - otherwise set to 0
					slopes[j] = slope > 0.0f ? slope : 0.0f;
					slope_sum += slopes[j];
				}

				if (slope_sum > 0.0f)
				{
					// determine transfer to each neighbor minus heat loss (dependent on slope)
					auto total_transfer = 0.0f;
					for (auto j = 0u; j < slopes.size(); j++)
					{
						if (slopes[j] > 0.0f)
						{
							auto transfer = (slopes[j] / slope_sum) * cell.temp;
							total_transfer += transfer;
							// auto loss = randf(0.0f, transfer_loss * elevation_step / slopes[j]);
							auto loss = transfer_loss * elevation_step / slopes[j];
							neighbors[j]->delta += transfer * (1.0f - loss);
						}
					}

					cell.temp -= total_transfer;
				}

				if (cell.emitter && cell.temp > emitter_capacity)
				{
					float c = std::min(1.0f, cell.temp);
					surface->set_pixel(i, texture_height - (int)(cell.z * (float)texture_height) - 1,
						Color{ c, 0, 0, 1.0f });

					cell.z += elevation_step;
					cell.temp *= 0.25f;
				}
				else if (!cell.emitter && cell.temp > capacity)
				{
					float c = cell.temp * 0.5f;
					surface->set_pixel(i, texture_height - (int)(cell.z * (float)texture_height) - 1,
						Color{ c, c, c, 1.0f });

					cell.z += elevation_step;
					cell.temp -= 0.5f * capacity;
				}
			}
			else
			{
				cell.veg += growth_rate * delta;
			}

			// temperature dissipates
			cell.temp -= dissipate_loss * delta;

			if (!cell.emitter)
			{
				cell.temp = std::min(1.0f, std::max(0.0f, cell.temp));
				cell.veg = std::min(1.0f, std::max(0.0f, cell.veg));
			}
		}

		// apply and reset deltas
		for (auto i = 0u; i < cells.size(); i++)
		{
			cells[i].temp += cells[i].delta;
			cells[i].delta = 0.0f;
		}
	}

	void Game::render_cells(void)
	{
		Color white{ 1.0f, 1.0f, 1.0f, 1.0f },
			black{ 0.0f, 0.0f, 0.0f, 1.0f };


#ifdef TOPDOWN
		for (int y = 0; y < texture_height; y++) 
		{
			for (int x = 0; x < texture_width; x++)
			{
				const auto& c = cells[y * texture_width + x];
				if (c.emitter)
				{
					if (c.active)
					{
						surface->set_pixel(x, y, white);
					}
					else
					{
						surface->set_pixel(x, y, black);
					}
				}
				else if (c.veg > 0.0f)
				{
					surface->set_pixel(x, y, Color{ 0.0f, c.veg, 0.0f, 1.0f });
				}
				else
				{
					surface->set_pixel(x, y, Color{ c.temp, 0.0f, 0.0f, 1.0f });
				}
			}
		}
#else
		for (int i = 0; i < texture_width; i++)
		{
			if (cells[i].emitter)
			{
				if (cells[i].active)
				{
					surface->set_pixel(i, texture_height - (int)(cells[i].z * (float)texture_height) - 1,
						Color{ 1.0f, 1.0f, 1.0f, 1.0f });
				}
				else
				{
					surface->set_pixel(i, texture_height - (int)(cells[i].z * (float)texture_height) - 1,
						Color{ 0.0f, 0.0f, 0.0f, 1.0f });
				}
			}
			else if (cells[i].veg > 0.0f)
			{
				surface->set_pixel(i, texture_height - (int)(cells[i].z * (float)texture_height) - 1,
					Color{ 0.0f, cells[i].veg, 0.0f, 1.0f });
			}
			else
			{
				surface->set_pixel(i, texture_height - (int)(cells[i].z * (float)texture_height) - 1,
					Color{ cells[i].temp, 0.0f, 0.0f, 1.0f });
			}
		}
#endif
	}

	void Game::update_debug_text(void)
	{
		std::stringstream ss;
		auto cam = renderer->get_camera();
		ss << "WIN: " << window->get_width() << "x" << window->get_height()
			<< " VIR: " << cam->transform.dimension.x << "x" << cam->transform.dimension.y
			<< " FPS: " << get_fps()
			<< " MESH: " << dukat::perfc.avg(dukat::PerformanceCounter::MESHES)
			<< " VERT: " << dukat::perfc.avg(dukat::PerformanceCounter::VERTICES) << std::endl;
		debug_text->set_text(ss.str());
	}
	
}

int main(int argc, char** argv)
{
	try
	{
		std::string config = "../assets/heatmap.ini";
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