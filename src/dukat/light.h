#pragma once

#include <gl/glew.h>
#include "color.h"
#include "vector3.h"

namespace dukat
{
	struct Light
	{
		Vector3 position;
		Color color;
		GLfloat attenuation;
		GLfloat ambient; // coefficient
	};
}