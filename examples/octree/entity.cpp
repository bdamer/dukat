#include "stdafx.h"
#include "entity.h"

namespace dukat
{
    void Entity::update(float delta)
    {
        transform.update();
        bb_world = bb_model->transform(transform.mat_model);
        mr_inv = transform.mat_rot.inverse();
    }

    float Entity::intersects(const Ray3& ray, float near_z, float far_z) const
    {
        return bb_world->intersect_ray(ray, near_z, far_z);
    }

    SDL_Color* Entity::sample(const Ray3& ray, float near_z, float far_z) const
    {
        return sample_model(Ray3(point_to_model(ray.origin), dir_to_model(ray.dir)));
    }

    SDL_Color* Entity::sample_model(Ray3 r) const
    {
        int oidx = 0;	// octant index

        // fixes for rs with negative direction
        if (r.dir.x == 0.0f) 
        {
            r.dir.x = small_number;
        }
        else if (r.dir.x < 0)
        {
            r.origin.x = 2.0f * this->root->origin.x - r.origin.x;		// why 2 * origin? I don't know. it works...http://kaba.hilvi.org/news/octree.hpp
                                                                        // but essentially, this moves the observer origin so that negative direction
                                                                        // does not screw up the internal order of possible nodes
            r.dir.x = -r.dir.x;
            oidx |= 4 ; // bitwise OR (last bits are XYZ)
        }
        
        if (r.dir.y == 0.0f) 
        {
            r.dir.y = small_number;
        }
        else if(r.dir.y < 0)
        {
            r.origin.y = 2.0f * this->root->origin.y - r.origin.y;		
            r.dir.y = -r.dir.y;
            oidx |= 2 ; 
        }

        if (r.dir.z == 0.0f) 
        {
            r.dir.z = small_number;
        }
        else if(r.dir.z < 0)
        {
            r.origin.z = 2.0f * this->root->origin.z - r.origin.z;
            r.dir.z = -r.dir.z;
            oidx |= 1 ; 
        }

        // Compute parametric entry / exit points of root node bounding box
        const auto& o = root->origin;
        const auto h = root->half_size;
        const auto inv_x = 1.0f / r.dir.x;
        const auto inv_y = 1.0f / r.dir.y;
        const auto inv_z = 1.0f / r.dir.z;
        const auto tx0 = (o.x - h - r.origin.x) * inv_x;
        const auto ty0 = (o.y - h - r.origin.y) * inv_y;
        const auto tz0 = (o.z - h - r.origin.z) * inv_z;
        const auto tx1 = (o.x + h - r.origin.x) * inv_x;
        const auto ty1 = (o.y + h - r.origin.y) * inv_y;
        const auto tz1 = (o.z + h - r.origin.z) * inv_z;

        if (std::max(tx0, std::max(ty0, tz0)) < std::min(tx1, std::min(ty1, tz1)))
        {
            return root->sample(tx0, ty0, tz0, tx1, ty1, tz1, oidx);
        }
        else
        {
            return nullptr;
        }
    }
}