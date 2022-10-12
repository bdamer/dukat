#include "stdafx.h"
#include <dukat/log.h>
#include <dukat/sprite.h>
#include <dukat/texturecache.h>
#include <dukat/rect.h>
#include <dukat/matrix4.h>
#include <dukat/vector3.h>
#include <dukat/meshdata.h>
#include <dukat/renderer2.h>

namespace dukat
{
	MemoryPool<Sprite> Sprite::_pool(2048);

	Sprite::Sprite(Texture* texture) : cols(1), rows(1), p(0, 0), z(0), custom(), 
		scale(1), rot(0), color({ 1.0f, 1.0f, 1.0f, 1.0f }), index(0), flags(0)
	{
		w = texture->w;
		h = texture->h;
		texture_id = texture->id;
		tex[0] = 0.0f;
		tex[1] = 0.0f;
		tex[2] = 1.0f;
		tex[3] = 1.0f;
	}

	Sprite::Sprite(Texture* texture, Rect rect) : cols(1), rows(1), p(0, 0), z(0), custom(),
		scale(1), rot(0), color({ 1.0f, 1.0f, 1.0f, 1.0f }), index(0), flags(0)
	{
		w = rect.w;
		h = rect.h;
		texture_id = texture->id;

		// Calculate texture coordinates based on size of sprite and texture atlas
		tex[0] = static_cast<float>(rect.x) / static_cast<float>(texture->w);
		tex[1] = static_cast<float>(rect.y) / static_cast<float>(texture->h);
		tex[2] = static_cast<float>(rect.w) / static_cast<float>(texture->w);
		tex[3] = static_cast<float>(rect.h) / static_cast<float>(texture->h);
	}

	Sprite::Sprite(Texture* texture, int cols, int rows) : cols(cols), rows(rows), p(0, 0), z(0), custom(), 
		scale(1), rot(0), color({ 1.0f, 1.0f, 1.0f, 1.0f }), index(0), flags(0)
	{
		// determine the dimensions of each map entry
		Rect rect = { 0, 0, static_cast<int>(texture->w) / cols, static_cast<int>(texture->h) / rows };
		w = rect.w;
		h = rect.h;
		texture_id = texture->id;
		// Calculate texture coordinates based on size of sprite and texture atlas
		tex[0] = 0.0f;
		tex[1] = 0.0f;
		tex[2] = static_cast<float>(rect.w) / static_cast<float>(texture->w);
		tex[3] = static_cast<float>(rect.h) / static_cast<float>(texture->h);
	}

	void Sprite::set_index(int index)
	{
		this->index = index;
		// change texture offset to match new index
		tex[0] = (index % cols) * tex[2];
		tex[1] = (index / cols) * tex[3];
	}

	Sprite& Sprite::operator++(void)
	{
		if (++index > (cols * rows - 1))
			index = 0;
		// change texture offset to match new index
		tex[0] = (index % cols) * tex[2];
		tex[1] = (index / cols) * tex[3];
		return *this;
	}

	Sprite& Sprite::operator--(void)
	{
		if (--index < 0)
			index = cols * rows - 1;
		// change texture offset to match new index
		tex[0] = (index % cols) * tex[2];
		tex[1] = (index / cols) * tex[3];
		return *this;
	}

	AABB2 compute_sprite_bb(const Sprite& sprite)
	{
		const Vector2 half_dim(sprite.scale * sprite.w / 2.0f, sprite.scale * sprite.h / 2.0f);

		// Adjust center pos based on alignment
		auto pos = sprite.p;
		const auto center = sprite.flags
			& (Sprite::align_bottom | Sprite::align_left | Sprite::align_right | Sprite::align_top);
		if (center > 0)
		{
			if (check_flag(center, Sprite::align_bottom))
				pos.y -= half_dim.y;
			else if (check_flag(center, Sprite::align_top))
				pos.y += half_dim.y;
			if (check_flag(center, Sprite::align_right))
				pos.x -= half_dim.x;
			else if (check_flag(center, Sprite::align_left))
				pos.x += half_dim.x;
		}

		// If sprite is axis-aligned, we can use top-left and bottom-right to construct BB
		if (sprite.rot == 0.0f)
		{
			return AABB2(pos - half_dim, pos + half_dim);
		}
		// Otherwise, need to rotate all 4 corners
		else
		{
			auto rotated = half_dim.rotate(sprite.rot);
			AABB2 bb;
			bb.add(pos + Vector2{ -rotated.x, -rotated.y });
			bb.add(pos + Vector2{ rotated.x, -rotated.y });
			bb.add(pos + Vector2{ -rotated.x, rotated.y });
			bb.add(pos + Vector2{ rotated.x, rotated.y });
			return bb;
		}
	}
}
