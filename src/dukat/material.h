#pragma once

#include "color.h"

namespace dukat
{
	struct Material
	{
		// Material color components
		Color ambient;
		Color diffuse;
		Color specular;
		// Reserved space for custom properties
		Color custom;

		Material(void) : ambient({1.0f, 1.0f, 1.0f, 1.0f}), diffuse({1.0f, 1.0f, 1.0f, 1.0f}), 
			specular({ 1.0f, 1.0f, 1.0f, 1.0f }), custom({ 1.0f, 1.0f, 1.0f, 1.0f }) { }
	};
}