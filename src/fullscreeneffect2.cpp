#include "stdafx.h"
#include <dukat/fullscreeneffect2.h>
#include <dukat/animationmanager.h>
#include <dukat/game2.h>

namespace dukat
{
    FullscreenEffect2::FullscreenEffect2(Game2* game) : game(game), anim(nullptr), color({0.f, 0.f, 0.f, 0.f}), alpha(0.f)
    {
    }

    FullscreenEffect2::~FullscreenEffect2(void)
    {
        if (anim != nullptr)
            game->get<AnimationManager>()->cancel(anim);
        auto renderer = game->get_renderer();
        auto layer = renderer->get_layer(fade_layer);
        if (layer != nullptr)
            renderer->destroy_layer(fade_layer);
    }

    void FullscreenEffect2::create_fade_layer(void)
    {
        auto renderer = game->get_renderer();
        auto layer = renderer->get_layer(fade_layer);
        if (layer == nullptr)
            layer = renderer->create_overlay_layer(fade_layer, layer_priority);	
        else 
            layer->show();
        layer->set_composite_program(game->get_shaders()->get_program("fx_default.vsh", "fx_solid.fsh"));
        layer->set_composite_binder([&](ShaderProgram* p) {
            p->set("u_diffuse", color.r, color.g, color.b, color.a * alpha);
        });
    }

    void FullscreenEffect2::fade_in(float duration, std::function<void(void)> callback)
    {
        create_fade_layer();
        alpha = 1.0f;
        auto value_anim = std::make_unique<ValueAnimation<float>>(&alpha, duration, 0.0f);
        value_anim->set_callback([&, callback](void) {
            if (callback != nullptr)
                callback();
            anim = nullptr;
        });
        anim = game->get<AnimationManager>()->add(std::move(value_anim));
    }

    void FullscreenEffect2::fade_out(float duration, std::function<void(void)> callback)
    {
        create_fade_layer();
        alpha = 0.0f;
        auto value_anim = std::make_unique<ValueAnimation<float>>(&alpha, duration, 1.0f);
        value_anim->set_callback([&, callback](void) {
            if (callback != nullptr)
                callback();
            anim = nullptr;
        });
        anim = game->get<AnimationManager>()->add(std::move(value_anim));
    }
}