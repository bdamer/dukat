#pragma once

#include <memory>
#include <dukat/game2.h>

namespace dukat
{
	class Game : public Game2
	{
	private:
		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;

		void init(void);
		void update_debug_text(void);

	public:
		Game(Settings& settings) : Game2(settings) { }
		~Game(void) { }
	};
}