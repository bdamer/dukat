#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class RipplePondScene : public Scene, public Controller
	{
	private:
		static constexpr auto texture_size = 256;
		static constexpr auto ripple_amplitude = 1000.0f;
		static constexpr auto ripple_dampening = 0.015f;
		static constexpr auto refraction_index = 2.0f; // water

		Game3* game;
		MeshGroup overlay_meshes;
		MeshInstance* quad_mesh;
		MeshInstance* info_mesh;

		std::unique_ptr<Texture> tex0;
		std::unique_ptr<Texture> tex1;
		std::array<Texture*,5> background;
		int background_idx;
		Texture* current;
		Texture* last;

        std::unique_ptr<FrameBuffer> fbo; // frame buffer object
		ShaderProgram* fb_program; // program used during framebuffer render pass

		void add_ripple(int x, int y, float amplitude);

	public:
		RipplePondScene(Game3* game);
		~RipplePondScene(void) { }

		void activate(void);
		void update(float delta);
		void update_framebuffer(float delta);
		void render(void);
		void handle_keyboard(const SDL_Event& e);
	};
}