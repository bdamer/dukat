#pragma once

#include <memory>

namespace dukat
{
	class Model3;

	// Abstract base class for model loaders. 
	class ModelConverter
	{
	public:
		ModelConverter(void) { }
		virtual ~ModelConverter(void) { }

		virtual std::unique_ptr<Model3> convert(void) = 0;
	};
}