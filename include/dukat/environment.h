#pragma once

#include "color.h"
#include "light.h"
#include "meshinstance.h"
#include <functional>
#include <vector>

namespace dukat
{
    class Game3;

    class Environment
    {
    public:
        // Environment phase determines lighting and shading of skydome
        struct Phase
        {
            float duration; // phase duration in seconds
            Material mat; // material used to color skydome
                          // ambient Horizon color
                          // diffuse Zenith color
                          // specular Corona color
                          // custom.r Extent of corona (large values decrease extent)
            Light3 light; // directional light

            std::function<void(float,float,const Color&, const Color&, Color&)> color_trans; // transition for colors
            std::function<void(float,float,const Vector3&, const Vector3&, Vector3&)> light_pos_trans; // transition for light position
        };

    private:
        Game3* game;
        MeshInstance* skybox;

        int cur_phase;
        float cur_duration;
        std::vector<Phase> phases;

    public:
        Environment(Game3* game, MeshInstance* skybox) : game(game), skybox(skybox), cur_phase(-1), cur_duration(0.0f) { }
        ~Environment(void) { }

        void update(float delta);

        void set_current_phase(int p) { this->cur_phase = p; }
        int get_current_phase(void) const { return cur_phase; } 
        int get_phase_count(void) const { return (int)phases.size(); }
        Phase& get_phase(int idx) { return phases[idx]; }
        void set_phase(int idx, const Phase& phase) { phases.insert(phases.begin() + idx, phase); }
        void add_phase(const Phase& phase) { phases.push_back(phase); }
        void remove_phase(int idx) { phases.erase(phases.begin() + idx); }
    };
}