#include "stdafx.h"
#include "log.h"
#include "sprite.h"
#include "texturecache.h"
#include "rect.h"
#include "matrix4.h"
#include "vector3.h"
#include "renderer2.h"

namespace dukat
{
	Sprite::Sprite(Texture* texture) : p(0, 0), z(0), center(0), scale(1), rot(0), color({ 1.0f, 1.0f, 1.0f, 1.0f }),
		index(0), COLS(1), ROWS(1), normal_id(0), rendered(true), pixel_aligned(false), relative(false)
	{
		w = texture->w;
		h = texture->h;
		texture_id = texture->id;
		tex[0] = 0.0f;
		tex[1] = 0.0f;
		tex[2] = 1.0f;
		tex[3] = 1.0f;
	}

	Sprite::Sprite(Texture* texture, Rect rect) : p(0, 0), z(0), center(0), scale(1), rot(0), color({ 1.0f, 1.0f, 1.0f, 1.0f }),
		index(0), COLS(1), ROWS(1), normal_id(0), rendered(true), pixel_aligned(false), relative(false)
	{
		w = rect.w;
		h = rect.h;
		texture_id = texture->id;
		// Calculate texture coordinates based on size of sprite and texture atlas
		tex[0] = (float)rect.x / (float)texture->w;
		tex[1] = (float)rect.y / (float)texture->h;
		tex[2] = (float)(rect.w) / (float)texture->w;
		tex[3] = (float)(rect.h) / (float)texture->h;
	}

	Sprite::Sprite(Texture* texture, int cols, int rows) : p(0, 0), z(0), center(0), scale(1), rot(0), color({ 1.0f, 1.0f, 1.0f, 1.0f }),
		index(0), COLS(cols), ROWS(rows), normal_id(0), rendered(true), pixel_aligned(false), relative(false)
	{
		// determine the dimensions of each map entry
		Rect rect = { 0, 0, (int)texture->w / cols, (int)texture->h / rows };
		w = rect.w;
		h = rect.h;
		texture_id = texture->id;
		// Calculate texture coordinates based on size of sprite and texture atlas
		tex[0] = 0.0f;
		tex[1] = 0.0f;
		tex[2] = (float)(rect.w) / (float)texture->w;
		tex[3] = (float)(rect.h) / (float)texture->h;
	}

	void Sprite::set_index(int index)
	{
		this->index = index;
		// change texture offset to match new index
		tex[0] = (index % COLS) * tex[2];
		tex[1] = (index / COLS) * tex[3];
	}

	Sprite& Sprite::operator++(void)
	{
		if (++index > (COLS * ROWS - 1))
			index = 0;
		// change texture offset to match new index
		tex[0] = (index % COLS) * tex[2];
		tex[1] = (index / COLS) * tex[3];
		return *this;
	}

	Sprite& Sprite::operator--(void)
	{
		if (--index < 0)
			index = COLS * ROWS - 1;
		// change texture offset to match new index
		tex[0] = (index % COLS) * tex[2];
		tex[1] = (index / COLS) * tex[3];
		return *this;
	}
}
