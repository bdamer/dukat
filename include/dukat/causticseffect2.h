#pragma once

#include "effect2.h"

namespace dukat
{
	class Game2;
	class ShaderProgram;
	struct FrameBuffer;
	class MeshData;
	struct Texture;

	// Effect to render caustic patterns to a texture.
	class CausticsEffect2 : public Effect2
	{
	private:
		Game2* game;
		ShaderProgram* sp;
		std::unique_ptr<FrameBuffer> fbo; // frame buffer object
		std::unique_ptr<MeshData> fb_quad; // quad mesh used during framebuffer render pass

		int color_set;
		float velocity; // velocity of change

	public:
		CausticsEffect2(Game2* game, ShaderProgram* sp, int width, int height, float velocity);
		~CausticsEffect2(void) { };

		void resize(int width, int height);
		void render(Renderer2* renderer, const AABB2& camera_bb);
	
		Texture* get_texture(void) const;
	};
}