#include "stdafx.h"
#include <dukat/scene2.h>
#include <dukat/game2.h>

namespace dukat
{
	void Scene2::render(void)
	{
		game->get_renderer()->render();
	}
}