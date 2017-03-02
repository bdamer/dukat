#pragma once

#include <memory>
#include <map>

#include "meshinstance.h"
#include "renderable.h"
#include "renderer3.h"
#include "aabb3.h"

namespace dukat
{
	class MeshGroup : public Renderable
	{
	private:
		std::vector<std::unique_ptr<MeshInstance>> instances;

	public:
        AABB3 bb;
		Matrix4 mat_model;

		MeshGroup(void);
		~MeshGroup(void) { }

		MeshInstance* create_instance(void) { instances.push_back(std::make_unique<MeshInstance>()); return instances.back().get(); }
		MeshInstance* add_instance(std::unique_ptr<MeshInstance> instance);
        void remove_instance(MeshInstance* instance);
		MeshInstance* get_instance(unsigned int index) { return instances[index].get(); }
		unsigned int size(void) { return static_cast<unsigned int>(instances.size()); }

		void update(float delta);
		// TODO: revisit - we may want to batch these calls
		void render(Renderer3* renderer);
	};
}