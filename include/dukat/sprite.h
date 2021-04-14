#pragma once

#include <memory>
#include "color.h"
#include "texturecache.h"
#include "vector2.h"

namespace dukat
{
	struct Rect;
	class Matrix4;

	struct Sprite
	{
		enum Flags
		{
			align_bottom = 1,	// alignment
			align_right = 2,
			align_top = 4,
			align_left = 8,
			flip_h = 16,		// flip
			flip_v = 32,
			relative = 64,		// if set, will interpret position relative to camera
			fx = 128			// can be used by custom render effects
		};

		// Number of columns and rows in this sprite
		const int cols, rows;
		// Position on screen
		Vector2 p;
		// Rendering priority
		GLfloat z;
		// Dimension in pixels
		int w, h;
		// Texture coordinates
		GLfloat tex[4];
		// Scaling factor
		GLfloat scale;
		GLfloat rot;
		// Color and opacity
		Color color;
		TextureId texture_id;
		TextureId normal_id;
		// current index for sprite map.
		int index;
		// Sprite flags
		uint8_t flags;

		// Default constructor
		Sprite(void) : cols(1), rows(1), p(0, 0), z(0), w(0), h(0), scale(1), rot(0), color({ 1.0f, 1.0f, 1.0f, 1.0f }),
			texture_id(0), normal_id(0), index(0), flags(0) { }
		// Creates a sprite with the full size of the texture
		Sprite(Texture* texture);
		// Creates a sprite from a texture with the dimensions specified by r (in pixels)
		Sprite(Texture* texture, Rect r);
		// Creates a sprite map from a texture with cols * rows elements
		Sprite(Texture* texture, int cols, int rows);
		~Sprite(void) { }
		
		// Sprite map functions
		void set_index(int index);

		// Operator to decrement / increment index for sprite map
		Sprite& operator++(void);
		Sprite& operator--(void);

		bool operator<(const Sprite& s) const { return z < s.z; }
		bool operator>(const Sprite& s) const { return z > s.z; }
	};

	// Used to order entities by z value. For sprites with the same z value,
	// this will additionally order by texture ID to reduce the number of
	// texture switches necessary.
	struct SpriteComparator
	{
		bool operator()(const Sprite& lhs, const Sprite& rhs)
		{
			if (lhs.z == rhs.z)
			{
				return lhs.texture_id > rhs.texture_id;
			}
			else
			{
				return lhs.z > rhs.z;
			}
		}
		bool operator()(const std::shared_ptr<Sprite>& lhs, const std::shared_ptr<Sprite>& rhs)
		{
			if (lhs->z == rhs->z)
			{
				return lhs->texture_id > rhs->texture_id;
			}
			else
			{
				return lhs->z > rhs->z;
			}
		}
		bool operator()(Sprite* lhs, Sprite* rhs)
		{
			if (lhs->z == rhs->z)
			{
				return lhs->texture_id > rhs->texture_id;
			}
			else
			{
				return lhs->z > rhs->z;
			}
		}
	};
}