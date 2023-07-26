#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
    struct Ray3;

    class Entity 
    {
    private:
        std::unique_ptr<OctreeNode<SDL_Color>> root;
        // Bounding body in model space
        std::unique_ptr<BoundingBody3> bb_model;
        // Bounding body in world space 
        std::unique_ptr<BoundingBody3> bb_world;
        // inverse of rotation matrix
        Matrix4 mr_inv;

    public:
        ExtendedTransform3 transform;

        Entity(void) { }
        ~Entity(void) { }

        void update(float delta);
        // Test for intersection against this entity's bounding body.
        float intersects(const Ray3& ray, float near, float far) const;
        // Samples this entity along a given ray.
        SDL_Color* sample(const Ray3& ray, float near, float far) const;

        void set_bb(std::unique_ptr<BoundingBody3> bb) { this->bb_model = std::move(bb); }
        void set_octree(std::unique_ptr<OctreeNode<SDL_Color>> root) { this->root = std::move(root); }
        std::unique_ptr<OctreeNode<SDL_Color>> get_octree(void) { return std::move(root); }
    };
}