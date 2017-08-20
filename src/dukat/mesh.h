#pragma once

#include "transform3.h"
#include "matrix4.h"

namespace dukat
{
    class Renderer;

	enum RenderStage
	{
        SCENE,      // scene geometry
		OVERLAY     // no depth-buffer, used for UI
	};

    // Abstract base class for objects that can be rendered.
    class Mesh
    {
    public:
		ExtendedTransform3 transform;
		RenderStage stage;
        bool visible;

        Mesh(void) : stage(RenderStage::SCENE), visible(true) { };
        virtual ~Mesh(void) { };

        // Updates mesh transform.
        // Dynamically generated meshes can perform additional work in this method.
        virtual void update(float delta) = 0;
        // Renders this mesh. 
        virtual void render(Renderer* renderer) = 0;
    };
}