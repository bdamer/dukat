#pragma once

#include "camera2.h"
#include "aabb2.h"

namespace dukat
{
    template <typename T>
    class FollowerCamera2 : public Camera2
    {
    private:
        static constexpr auto jump_threshold = 10.0f;
        static constexpr auto lerp_speed = 32.0f;

		GameBase* game;
        T* target;
		Vector2 actual_pos; // unsmoothed position
        Vector2 actual_offset;
        Vector2 target_offset;
		float sharpness; // determines how closely we follow target [0..1]
		AABB2 limits; // limits camera movement

    public:
        FollowerCamera2(GameBase* game) : Camera2(game), game(game), target(nullptr), 
			actual_pos{ 0, 0 }, actual_offset{ 0, 0 }, target_offset{ 0, 0 }, sharpness(1.0f),
			limits(Vector2{ -big_number, -big_number }, Vector2{ big_number, big_number }) { }
        ~FollowerCamera2(void) { }

        void set_target(T* target, bool immediate = true);
        void set_offset(const Vector2& offset, bool immediate = true);
		void set_limits(const AABB2& bb) { this->limits = bb; }
		void set_sharpness(float sharpness) { this->sharpness = sharpness; }
        void update(float delta);
    };

    template <typename T>
    void FollowerCamera2<T>::set_target(T* target, bool immediate)
    {
		this->target = target;
		if (immediate)
			actual_pos = transform.position = (target->get_world_pos() + actual_offset);
    }

	template <typename T>
	void FollowerCamera2<T>::set_offset(const Vector2& offset, bool immediate)
	{
		if (immediate)
			actual_offset = target_offset = offset;
		else
			target_offset = offset;
	}

	template <typename T>
	void FollowerCamera2<T>::update(float delta)
	{
		if (target == nullptr)
			return;

		if (actual_offset != target_offset)
		{
			auto diff = target_offset - actual_offset;
			auto mag = diff.mag();
			if (mag <= 1.0f)
			{
				actual_offset = target_offset;
			}
			else
			{
				actual_offset += diff * (lerp_speed * delta / mag);
			}
		}

		auto target_pos = target->get_world_pos() + actual_offset;
		const auto blend = 1.f - std::pow(1.f - sharpness, delta * 30.f);
		actual_pos = lerp(actual_pos, target_pos, blend);

		// Round coordinates to nearest (sub)pixel. 
		auto pos = Vector2{ std::round(actual_pos.x * mag_factor) / mag_factor, std::round(actual_pos.y * mag_factor) / mag_factor };
		auto dx = target_pos.x - transform.position.x;
		auto dy = target_pos.y - transform.position.y;

		// Limit coordinates
		transform.position.x = std::max(std::min(pos.x, limits.max().x), limits.min().x);
		transform.position.y = std::max(std::min(pos.y, limits.max().y), limits.min().y);

		if (std::abs(dx) > jump_threshold || std::abs(dy) > jump_threshold)
		{
			// camera jump detected, trigger notification
			game->trigger(Message{ Events::CameraChanged });
		}

		Camera2::update(delta);
	}
}
