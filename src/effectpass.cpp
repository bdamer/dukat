#include "stdafx.h"
#include <dukat/effectpass.h>
#include <dukat/buffers.h>
#include <dukat/meshbuilder2.h>
#include <dukat/meshdata.h>
#include <dukat/shaderprogram.h>

namespace dukat
{
    EffectPass::EffectPass(void) : disable_blend(true)
    {
		for (auto i = 0; i < Renderer::max_texture_units; i++)
            texture[i] = nullptr;
        // TODO: get reference from cache instead?
		MeshBuilder2 builder;
		mesh = builder.build_textured_quad();
    }

    void EffectPass::set_texture(Texture* texture, int index)
	{
		assert(index >= 0 && index < Renderer::max_texture_units);
		this->texture[index] = texture;
	}

    void EffectPass::render(Renderer* renderer)
    {
        if (disable_blend)
        {
            // Disable blending so we can use ALPHA channel for map flags
			renderer->set_blending(false);
        }

        fbo->bind();
        renderer->switch_shader(program);

        // Bind textures
		for (auto i = 0; i < Renderer::max_texture_units; i++)
		{
			if (texture[i] != nullptr)
			{
				texture[i]->bind(i, program);
			}
		}

        // Bind attributes
        for (auto it = attributes.begin(); it != attributes.end(); ++it)
        {
            switch (it->second.size())
            {
            case 1:
                program->set(it->first, it->second[0]);
                break;            
            case 2:
                program->set(it->first, it->second[0], it->second[1]);
                break;            
            case 3:
                program->set(it->first, it->second[0], it->second[1], it->second[2]);
                break;
            case 4:
                program->set(it->first, it->second[0], it->second[1], it->second[2], it->second[3]);
                break;
            case 16:
                program->set_matrix4(it->first, it->second.data());
                break;
            }
        }

        mesh->render(program);
        perfc.inc(PerformanceCounter::FRAME_BUFFERS);
        
        fbo->unbind();

		renderer->set_blending(true);
    }
}