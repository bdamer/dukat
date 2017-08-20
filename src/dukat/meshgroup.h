#pragma once

#include <memory>
#include <map>

#include "meshinstance.h"
#include "mesh.h"
#include "renderer.h"
#include "aabb3.h"

namespace dukat
{
	class MeshGroup : public Mesh
	{
	private:
		std::vector<std::unique_ptr<MeshInstance>> instances;

	public:
        AABB3 bb;

		MeshGroup(void);
		~MeshGroup(void) { }

		MeshInstance* create_instance(void) { instances.push_back(std::make_unique<MeshInstance>()); return instances.back().get(); }
		MeshInstance* add_instance(std::unique_ptr<MeshInstance> instance);
        void remove_instance(MeshInstance* instance);
		MeshInstance* get_instance(int index) { return instances[index].get(); }
		int size(void) { return static_cast<int>(instances.size()); }

		void update(float delta);
		// TODO: revisit - we may want to batch these calls
		void render(Renderer* renderer);
	};
}