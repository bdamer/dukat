#pragma once

#include <memory>
#include <dukat/game3.h>
#include <dukat/meshgroup.h>

namespace dukat
{
	class Game : public Game3
	{
	private:
		static constexpr int texture_size = 256;

		MeshGroup debug_meshes;
		MeshGroup overlay_meshes;
		MeshInstance* quad_mesh;
		MeshInstance* info_mesh;

        std::unique_ptr<FrameBuffer> fbo; // frame buffer object
        ShaderProgram* fb_program; // program used during framebuffer render pass
        std::unique_ptr<MeshData> fb_quad; // quad mesh used during framebuffer render pass
		std::unique_ptr<Texture> fb_texture; // texture written to during framebuffer pass

		void init(void);
		void update(float delta);
		void update_debug_text(void);
		void update_framebuffer(float delta);
		void render(void);
		void handle_keyboard(const SDL_Event& e);
		void release(void);

	public:
		Game(Settings& settings) : Game3(settings) { }
		~Game(void) { }

		void toggle_debug(void);
	};
}