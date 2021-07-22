#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class TextScene : public Scene2
	{
	private:
		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;

		void reset_text(void);

	public:
		TextScene(Game2* game);
		~TextScene(void) { }

		void update(float delta);
	};
}