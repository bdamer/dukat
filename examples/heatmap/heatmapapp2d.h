#pragma once

#include <memory>
#include <dukat/game2.h>

#define TOPDOWN

namespace dukat
{
	struct Cell
	{
		bool emitter;
		bool active;
		float z; // elevation
		float temp; // temperature [0..1]
		float delta; // change in temperature
		float veg; // amount of vegetation [0..1]

		Cell(void) : emitter(false), active(false), z(0.0f), temp(0.0f), delta(0.0f), veg(0.0f) { }
	};

	struct Texture;
	class Sprite;
	class Surface;

	class Game : public Game2
	{
	private:
		const int texture_width = 128;
		const int texture_height = 128;

		std::vector<Cell> cells;

		std::unique_ptr<Texture> texture;
		std::unique_ptr<Surface> surface;
		std::unique_ptr<Sprite> sprite;
		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;

		void init(void);
		void handle_keyboard(const SDL_Event& e);
		void update_debug_text(void);
		void update(float delta);
		void render(void);
		void render_cells(void);
		void reset(void);

	public:
		Game(Settings& settings) : Game2(settings) { }
		~Game(void) { }
	};
}