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
		Material(Color amb, Color diff, Color spec, Color cust) : ambient(amb), diffuse(diff),
			specular(spec), custom(cust) { }
	};

	// Material constants from http://devernay.free.fr/cours/opengl/materials.html
	const Material mat_jade{
		Color{ 0.135f, 0.2225f, 0.1575f, 1.0f },
		Color{ 0.54f, 0.89f, 0.63f, 1.0f },
		Color{ 0.316228f, 0.316228f, 0.316228f, 1.0f },
		Color{ 0.1f, 0.0f, 0.0f, 0.0f }
	};

	const Material mat_ruby{
		Color{ 0.1745f, 0.01175f, 0.01175f, 1.0f },
		Color{ 0.61424f, 0.04136f, 0.04136f, 1.0f },
		Color{ 0.727811f, 0.626959f, 0.626959f, 1.0f },
		Color{ 0.6f, 0.0f, 0.0f, 0.0f }
	};
	
	const Material mat_gold{
		Color{ 0.24725f, 0.1995f, 0.0745f, 1.0f },
		Color{ 0.75164f, 0.60648f, 0.22648f, 1.0f },
		Color{ 0.628281f, 0.555802f, 0.366065f, 1.0f },
		Color{ 0.4f, 0.0f, 0.0f, 0.0f }
	};

	const Material mat_white_rubber{
		Color{ 0.05f, 0.05f, 0.05f, 1.0f },
		Color{ 0.5f, 0.5f, 0.5f, 1.0f },
		Color{ 0.7f, 0.7f, 0.7f, 1.0f },
		Color{ 0.078125f, 0.0f, 0.0f, 0.0f }
	};
}