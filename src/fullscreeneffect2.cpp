#include "stdafx.h"
#include <dukat/fullscreeneffect2.h>
#include <dukat/animationmanager.h>
#include <dukat/game2.h>
#include <dukat/log.h>

namespace dukat
{
    FullscreenEffect2::FullscreenEffect2(Game2* game) : game(game)
    {
        static constexpr auto uniform_time = "u_time";
        static constexpr auto uniform_diffuse = "u_diffuse";

        composite_binder = [&](ShaderProgram* sp) {
            if (sp->attr(uniform_time) != -1)
                sp->set(uniform_time, this->game->get_time());
            if (sp->attr(uniform_diffuse) != -1)
                sp->set(uniform_diffuse, state.color.r, state.color.g, state.color.b, state.color.a * state.alpha);
        };
    }

    FullscreenEffect2::~FullscreenEffect2(void)
    {
        auto am = game->get<AnimationManager>();
        if (state.anim != nullptr)
            am->cancel(state.anim);
        for (auto& it : state_stash)
        {
            if (it.second.anim != nullptr)
                am->cancel(it.second.anim);
        }
    }

    void FullscreenEffect2::begin_fade_in(float duration, std::function<void(void)> callback)
    {
        log->debug("Begin fade-in: {}", duration);
        cancel_anim();

        state.alpha = 1.0f;
        auto sp = game->get_shaders()->get_program(default_vsh, solid_fsh);
        game->get_renderer()->set_composite_program(sp, composite_binder);

        begin_anim(duration, 0.0f, callback);
    }

    void FullscreenEffect2::begin_fade_out(float duration, std::function<void(void)> callback)
    {
        log->debug("Begin fade-out: {}", duration);
        cancel_anim();

        state.alpha = 0.0f;
        auto sp = game->get_shaders()->get_program(default_vsh, solid_fsh);
        game->get_renderer()->set_composite_program(sp, composite_binder);
        
        begin_anim(duration, 1.0f, callback);
    }

    void FullscreenEffect2::begin_anim(float duration, float target, std::function<void(void)> callback)
    {
        auto value_anim = std::make_unique<ValueAnimation<float>>(&state.alpha, duration, target);
        value_anim->set_callback([&, callback](void) {
            if (callback != nullptr)
                callback();
            state.anim = nullptr;
        });
        state.anim = game->get<AnimationManager>()->add(std::move(value_anim));
    }

    void FullscreenEffect2::cancel_anim(void)
    {
        if (state.anim != nullptr)
        {
            game->get<AnimationManager>()->cancel(state.anim);
            state.anim = nullptr;
        }
    }

    void FullscreenEffect2::stash(const std::string& id)
    {
        if (state.anim != nullptr)
            state.anim->pause();

        state_stash[id] = state;
    }

    void FullscreenEffect2::restore(const std::string& id)
    {
        if (!state_stash.count(id))
        {
            reset_composite_program(); // if no effect defined, just go to default
            return;
        }

        if (state.anim != nullptr)
            game->get<AnimationManager>()->cancel(state.anim);

        state = state_stash[id];
        state_stash.erase(id);

        game->get_renderer()->set_composite_program(state.sp, composite_binder);

        if (state.anim != nullptr)
            state.anim->resume();
    }
    
    void FullscreenEffect2::set_composite_program(ShaderProgram* sp, std::function<void(ShaderProgram*)> binder)
    {
        game->get_renderer()->set_composite_program(sp, binder != nullptr ? binder : composite_binder);
        state.sp = sp; // back up SP so we can restore if needed
    }
    
    void FullscreenEffect2::reset_composite_program(void)
    {
        state.sp = game->get_shaders()->get_program(default_vsh, default_fsh);
        game->get_renderer()->set_composite_program(state.sp);
        state.color = Color{ 1, 1, 1, 1 };
        state.alpha = 0.0f;
        state.anim = nullptr;
    }
}