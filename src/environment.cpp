#include "stdafx.h"
#include <dukat/environment.h>
#include <dukat/transition.h>
#include <dukat/game3.h>
#include <dukat/log.h>

namespace dukat
{
    void Environment::update(float delta)
    {
        if (cur_phase < 0)
            return;

        // check if we're at a phase transition
        const auto& phase = phases[cur_phase];
        cur_duration += delta;
        if (cur_duration >= phase.duration)
        {
            // transition to next phase
            cur_duration -= phase.duration;
            cur_phase++;
            if (cur_phase >= (int)phases.size())
            {
                cur_phase = 0;
            }
            log->debug("Switching phase: {}", cur_phase);
        }

        const auto& p0 = phases[cur_phase];
        const auto& p1 = (cur_phase < ((int)phases.size() - 1)) ? phases[cur_phase + 1] : phases[0];

        Material mat;
        p0.color_trans(cur_duration, p0.duration, p0.mat.ambient, p1.mat.ambient, mat.ambient);
        p0.color_trans(cur_duration, p0.duration, p0.mat.diffuse, p1.mat.diffuse, mat.diffuse);
        p0.color_trans(cur_duration, p0.duration, p0.mat.specular, p1.mat.specular, mat.specular);
        p0.color_trans(cur_duration, p0.duration, p0.mat.custom, p1.mat.custom, mat.custom);
        skybox->set_material(mat);

        auto light0 = game->get_renderer()->get_light(0);
        p0.light_pos_trans(cur_duration, p0.duration, p0.light.position, p1.light.position, light0->position);
        p0.color_trans(cur_duration, p0.duration, p0.light.ambient, p1.light.ambient, light0->ambient);
        p0.color_trans(cur_duration, p0.duration, p0.light.diffuse, p1.light.diffuse, light0->diffuse);
        p0.color_trans(cur_duration, p0.duration, p0.light.specular, p1.light.specular, light0->specular);
    }
}