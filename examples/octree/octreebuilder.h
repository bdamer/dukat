#pragma once

#include <memory>
#include <SDL2/SDL.h>
#include <dukat/dukat.h>

#ifdef NOISE_ENABLED
#include <libnoise/noise.h>
#include "noiseutils.h"
#endif

namespace dukat
{
    class OctreeBuilder
    {
    private:

    public:
        OctreeBuilder(void) { }
        ~OctreeBuilder(void) { }

        // Generates a new empty voxel chunk.
        std::unique_ptr<OctreeNode<SDL_Color>> build_empty(int size);
        // Generate a new voxel cube.
        std::unique_ptr<OctreeNode<SDL_Color>> build_cube(int size);
        // Generates a new voxel sphere.
        std::unique_ptr<OctreeNode<SDL_Color>> build_sphere(int radius);
#ifdef NOISE_ENABLED
        // Generates a new voxel planetoid.
        std::unique_ptr<OctreeNode<SDL_Color>> build_planetoid(int radius, int surface_height);
#endif
    };
}