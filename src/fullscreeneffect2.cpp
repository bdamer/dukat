#include "stdafx.h"
#include <dukat/fullscreeneffect2.h>
#include <dukat/animationmanager.h>
#include <dukat/game2.h>
#include <dukat/log.h>

namespace dukat
{
    FullscreenEffect2::FullscreenEffect2(Game2* game) : game(game), anim(nullptr), last_sp(nullptr), color({0.f, 0.f, 0.f, 0.f}), alpha(0.f)
    {
        static constexpr auto uniform_time = "u_time";
        static constexpr auto uniform_diffuse = "u_diffuse";

        composite_binder = [&](ShaderProgram* sp) {
            if (sp->attr(uniform_time) != -1)
                sp->set(uniform_time, this->game->get_time());
            if (sp->attr(uniform_diffuse) != -1)
                sp->set(uniform_diffuse, color.r, color.g, color.b, color.a * alpha);
        };
    }

    FullscreenEffect2::~FullscreenEffect2(void)
    {
        if (anim != nullptr)
            game->get<AnimationManager>()->cancel(anim);
    }

    void FullscreenEffect2::begin_fade_in(float duration, std::function<void(void)> callback)
    {
        log->debug("Begin fade-in: {}", duration);
        cancel_anim();

        alpha = 1.0f;
        auto sp = game->get_shaders()->get_program("fx_default.vsh", "fx_solid.fsh");
        game->get_renderer()->set_composite_program(sp, composite_binder);

        begin_anim(duration, 0.0f, callback);
    }

    void FullscreenEffect2::begin_fade_out(float duration, std::function<void(void)> callback)
    {
        log->debug("Begin fade-out: {}", duration);
        cancel_anim();

        alpha = 0.0f;
        auto sp = game->get_shaders()->get_program("fx_default.vsh", "fx_solid.fsh");
        game->get_renderer()->set_composite_program(sp, composite_binder);
        
        begin_anim(duration, 1.0f, callback);
    }

    void FullscreenEffect2::begin_anim(float duration, float target, std::function<void(void)> callback)
    {
        auto value_anim = std::make_unique<ValueAnimation<float>>(&alpha, duration, target);
        value_anim->set_callback([&, callback](void) {
            if (callback != nullptr)
                callback();
            anim = nullptr;
        });
        anim = game->get<AnimationManager>()->add(std::move(value_anim));
    }

    void FullscreenEffect2::cancel_anim(void)
    {
        if (anim != nullptr)
        {
            game->get<AnimationManager>()->cancel(anim);
            anim = nullptr;
        }
    }
    
    void FullscreenEffect2::set_composite_program(ShaderProgram* sp, std::function<void(ShaderProgram*)> binder)
    {
        game->get_renderer()->set_composite_program(sp, binder != nullptr ? binder : composite_binder);
        last_sp = sp; // back up SP so we can restore if needed
    }
    
    void FullscreenEffect2::reset_composite_program(void)
    {
        auto sp = game->get_shaders()->get_program(default_vsh, default_fsh);
        game->get_renderer()->set_composite_program(sp);
    }
    
    void FullscreenEffect2::restore_composite_program(void)
    {
        if (last_sp != nullptr)
            game->get_renderer()->set_composite_program(last_sp, composite_binder);
    }
}