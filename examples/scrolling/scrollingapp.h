#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	struct Texture;
	struct Sprite;
	class Surface;

	class ScrollingScene : public Scene2, public Controller
	{
	private:
		// Background tiles
		static constexpr auto tile_size = 16;
		static constexpr auto tile_rows = 14;
		static constexpr auto tile_cols = 24;
		static constexpr auto tile_scale = 1;
		static constexpr auto scaled_tile_size = tile_scale * tile_size;

		struct Player
		{
			Vector2 pos;
			const Vector2& get_world_pos(void) { return pos; }
		};

		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;
		std::vector<std::unique_ptr<Sprite>> background_sprites;
		std::unique_ptr<Sprite> player_sprite;
		std::unique_ptr<Player> player;
		std::unique_ptr<Sprite> heart_sprite;
		std::unique_ptr<Sprite> star_sprite;
		int world_x;

		void init_background(RenderLayer2* layer);
		void init_row(int index, Texture* tex, const Rect& rect, RenderLayer2* layer);

		void update_tiles(void);

	public:
		ScrollingScene(Game2* game2);
		~ScrollingScene(void) { }

		void handle_keyboard(const SDL_Event& e);
		void update(float delta);
	};
}