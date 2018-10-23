#include "stdafx.h"
#include "fixedcamera3.h"

namespace dukat
{    
    FixedCamera3::FixedCamera3(GameBase* game, const Vector3& eye, const Vector3& look_at, const Vector3& up)
		: Camera3(game->get_window()), min_distance(0.0f), max_distance(1000.0f), look_at(look_at)
    {
        transform.position = eye;
        transform.up = up;
    }

    void FixedCamera3::update(float delta)
    {
        // Dir is from eye to target
        transform.dir = look_at - transform.position;
        transform.dir.normalize();
        // Compute side from up and dir, then recompute up from right and dir
        transform.right = cross_product(transform.dir, transform.up);
        transform.right.normalize();
        transform.up = cross_product(transform.right, transform.dir);
        transform.up.normalize();
		Camera3::update(delta);
    }

	void FixedCamera3::set_look_at(const Vector3& look_at)
	{
		// store eye to look-at offset
		auto offset = transform.position - this->look_at;
		this->look_at = look_at;
		transform.position = this->look_at + offset;
    }
    
    void FixedCamera3::set_distance(float distance) 
    { 
        clamp(distance, min_distance, max_distance);
        transform.position = look_at - transform.dir * distance; 
    }
}