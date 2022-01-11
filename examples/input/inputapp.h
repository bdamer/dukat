#pragma once

#include <dukat/dukat.h>

namespace dukat
{
	struct Sprite;

	class InputScene : public Scene2, public Controller, public Recipient
	{
	private:
		std::unique_ptr<Sprite> mask_sprite;
		std::array<std::unique_ptr<Sprite>, 8> button_sprites;
		std::array<std::unique_ptr<Sprite>, 4> dir_sprites;
		std::array<std::unique_ptr<Sprite>, 4> left_sprites;
		std::array<std::unique_ptr<Sprite>, 4> right_sprites;
		std::unique_ptr<Sprite> select_sprite;
		std::unique_ptr<Sprite> start_sprite;
		std::unique_ptr<Sprite> ltrigger_sprite;
		std::unique_ptr<Sprite> rtrigger_sprite;
		FeedbackSequence* feedback;
		Animation* anim;
		float text_alpha;

		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> action_text;
		std::unique_ptr<TextMeshInstance> debug_text;

		void create_sprites(void);
		void bind_events(void);
		void update_info_text(void);
		void toggle_record(void);
		void toggle_replay(void);
		void start_animation(void);

		void feedback_heartbeat(void);
		void feedback_hi_lo(void);

	public:
		InputScene(Game2* game2);
		~InputScene(void);

		void handle_keyboard(const SDL_Event& e);
		void update(float delta);
		void receive(const Message& msg);
	};
}