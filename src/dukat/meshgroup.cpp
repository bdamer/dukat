#include "stdafx.h"
#include "meshgroup.h"
#include "renderer.h"

namespace dukat
{
	MeshGroup::MeshGroup(void)
	{ 
		bb.clear();
	}

	MeshInstance* MeshGroup::add_instance(std::unique_ptr<MeshInstance> instance)
	{
		auto res = instance.get();
		instances.push_back(std::move(instance));
		return res;
	}

	void MeshGroup::remove_instance(MeshInstance* instance)
    {
        auto it = std::find_if(instances.begin(), instances.end(), [instance](const std::unique_ptr<MeshInstance>& ptr) {
            return ptr.get() == instance;
        });
        if (it != instances.end())
        {
            instances.erase(it);
        }
    }

	void MeshGroup::update(float delta)
	{
		transform.update();
		for (auto& it : instances)
		{
			it->transform.update();
		}
	}

	void MeshGroup::render(Renderer* renderer)
	{
		for (auto& it : instances)
		{
			if (it->visible)
			{
				it->render(renderer, transform.mat_model);
			}
		}
	}
}
