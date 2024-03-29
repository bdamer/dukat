#pragma once

#include <memory>
#include "aabb2.h"
#include "color.h"
#include "memorypool.h"
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
		// Customizable values accessible in shader
		GLfloat custom[2];
		// Texture coordinates
		GLfloat tex[4];
		// Scaling factor
		GLfloat scale;
		GLfloat rot;
		// Color and opacity
		Color color;
		TextureId texture_id;
		// current index for sprite map.
		int index;
		// Sprite flags
		uint8_t flags;

		// Default constructor
		Sprite(void) : cols(1), rows(1), p(0, 0), z(0), w(0), h(0), custom(), tex(), scale(1), 
			rot(0), color({ 1.0f, 1.0f, 1.0f, 1.0f }), texture_id(0), index(0), flags(0) { }
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

		// Custom memory allocation
		static MemoryPool<Sprite> _pool;
		static void* operator new(std::size_t size) { return _pool.allocate(size); }
		static void operator delete(void* ptr, std::size_t size) { return _pool.free(ptr, size); }
	};

	extern AABB2 compute_sprite_bb(const Sprite& sprite);

	// Used to order entities by z value. For sprites with the same z value,
	// this will additionally order by texture ID to reduce the number of
	// texture switches necessary; if texture ID is the same, will finally
	// use memory address to keep ordering stable between subsequent invocations.
	template <typename T>
	struct SpriteComparator
	{
		bool operator()(const T& lhs, const T& rhs)
		{
			if (lhs->z == rhs->z)
			{
				if (lhs->texture_id != rhs->texture_id)
					return lhs->texture_id > rhs->texture_id;
				else
					return lhs > rhs;
			}
			else
			{
				return lhs->z > rhs->z;
			}
		}
	};
}