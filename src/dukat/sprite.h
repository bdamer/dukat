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
		typedef Uint8 Alignment;
		static constexpr Alignment align_center = 0;
		static constexpr Alignment align_bottom = 1;
		static constexpr Alignment align_right = 2;
		static constexpr Alignment align_top = 4;
		static constexpr Alignment align_left = 8;

		const int COLS, ROWS;
		// Position
		Vector2 p;
		// Rendering priority
		GLfloat z;
		// Dimension in pixels
		int w, h;
		// Texture coordinates
		GLfloat tex[4];
		// Sprite Alignment
		Alignment center;
		// Scaling factor
		GLfloat scale;
		GLfloat rot;
		// Color and opacity
		Color color;
		TextureId texture_id;
		TextureId normal_id;
		// current index for sprite map.
		int index;
		// If true will align sprite to pixel boundaries.
		bool pixel_aligned;
		// If true will interpret position relative to camera.
		bool relative;
		// True if sprite was rendered during last frame
		bool rendered;

		// Default constructor
		Sprite(void) : p(0, 0), z(0), w(0), h(0), center(0), scale(1), rot(0), color({ 1.0f, 1.0f, 1.0f, 1.0f }),
			texture_id(0), normal_id(0), index(0), COLS(1), ROWS(1), pixel_aligned(false), relative(false) { }
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