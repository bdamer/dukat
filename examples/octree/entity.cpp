#include "stdafx.h"
#include "entity.h"
#include <dukat/mathutil.h>
#include <dukat/perfcounter.h>
#include <dukat/ray3.h>

namespace dukat
{
    void Entity::update(float delta)
    {
        transform.update();
        bb_world = bb_model->transform(transform.mat_model);
        mr_inv = transform.mat_rot.inverse();
    }

    float Entity::intersects(const Ray3& ray, float near, float far) const
    {
        perfc.inc(PerformanceCounter::BB_CHECKS);
        return bb_world->intersect_ray(ray, near, far);
    }

    SDL_Color* Entity::sample(const Ray3& ray, float near, float far) const
    {
        perfc.inc(PerformanceCounter::SAMPLES);

        // We need to transpose the origin of the ray before applying the
        // inverse rotation. Dir can be immeditaly rotated, since it is a
        // direction vector.
        Ray3 r((ray.origin - transform.position) * mr_inv, ray.dir * mr_inv);

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

        auto inv_dir = r.dir.inverse();
        Vector3 half_size(this->root->half_size, this->root->half_size, this->root->half_size);
        auto t0 = (this->root->origin - half_size - r.origin);
        auto t1 = (this->root->origin + half_size - r.origin);
        t0.x *= inv_dir.x;
        t0.y *= inv_dir.y;
        t0.z *= inv_dir.z;
        t1.x *= inv_dir.x;
        t1.y *= inv_dir.y;
        t1.z *= inv_dir.z;

        if (t0.max_el() < t1.min_el())
        {
            return root->sample(t0.x, t0.y, t0.z, t1.x, t1.y, t1.z, oidx);
        }
        else
        {
            return nullptr;
        }

    }
}