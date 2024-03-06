#pragma once

#include <map>
#include "shaderprogram.h"

namespace dukat
{
	struct EffectParameter
	{
		int count;
		float values[4];

		EffectParameter(void) : count(0), values() {  }
		EffectParameter(float val0) : count(1) { values[0] = val0; }
		EffectParameter(float val0, float val1) : count(2) { values[0] = val0; values[1] = val1; }
		EffectParameter(float val0, float val1, float val2) : count(3) { values[0] = val0; values[1] = val1; values[2] = val2; }
		EffectParameter(float val0, float val1, float val2, float val3) : count(4) { values[0] = val0; values[1] = val1; values[2] = val2; values[3] = val3; }
	};

	struct Effect3
	{
		ShaderProgram* program;
		std::map<std::string, EffectParameter> parameters;
	};
}