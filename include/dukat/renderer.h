#pragma once

#include <string>

#ifndef OPENGL_VERSION
#include "version.h"
#endif // !OPENGL_VERSION

#include "recipient.h"
#include "window.h"

namespace dukat
{
	class ShaderCache;
	class ShaderProgram;
	struct GenericBuffer;
	struct Color;

	enum RenderStage
	{
		SCENE,      // scene geometry
		OVERLAY     // no depth-buffer, used for UI
	};

	// Base class for 2D and 3D renderers.
	class Renderer
	{
	protected:		
		Window* window;
		ShaderCache* shader_cache;
		// The currently active shader program
		ShaderProgram* active_program;
		// Uniform buffers
		std::unique_ptr<GenericBuffer> uniform_buffers;
		bool show_wireframe;
		bool backface_culling;
		bool blending;

		void test_capabilities(void);

	public:
		enum UniformBuffer
		{
			CAMERA,
			LIGHT,
			MATERIAL,
			USER,
			_COUNT
		};

		// Used to restart primitives in batched call
		static constexpr const GLushort primitive_restart = (GLushort)-1;

		// Limits
		static constexpr const int max_texture_units = 4;

		// Vertex attributes
		static constexpr const char* at_pos = "a_position";
		static constexpr const char* at_normal = "a_normal";
		static constexpr const char* at_color = "a_color";
		static constexpr const char* at_texcoord = "a_tex_coord";
		// Uniform variables
		static constexpr const char* uf_color = "u_color";
		static constexpr const char* uf_model = "u_model";
		static constexpr const char* uf_tex0 = "u_tex0";
		static constexpr const char* uf_tex1 = "u_tex1";
		static constexpr const char* uf_tex2 = "u_tex2";
		static constexpr const char* uf_tex3 = "u_tex3";
#if OPENGL_VERSION >= 30
		// Uniform blocks
		static constexpr const char* uf_camera = "Camera";
		static constexpr const char* uf_light = "Lights";
		static constexpr const char* uf_material = "Material";
#else
		// Uniforms to use instead of blocks for OpenGL 2
		static constexpr const char* u_cam_proj_pers = "u_cam_proj_pers";
		static constexpr const char* u_cam_proj_orth = "u_cam_proj_orth";
		static constexpr const char* u_cam_view = "u_cam_view";
		static constexpr const char* u_cam_view_inv = "u_cam_view_inv";
		static constexpr const char* u_cam_position = "u_cam_position";
#endif

		Renderer(Window* window, ShaderCache* shaders);
		virtual ~Renderer(void);

		// Display settings
		void toggle_wireframe(void) { set_wireframe(!show_wireframe); }
		void set_wireframe(bool wireframe);
		void set_backface_culling(bool backface_culling);
		void set_blending(bool blending);
		void set_clear_color(const Color& clr);

		// Clears screen buffers.
		void clear(void) { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }
		// Resets viewport to window size.
		void reset_viewport(void);
		// Changes active program.
		void switch_shader(ShaderProgram* program);
		// Returns the currently active program.
		ShaderProgram* get_active_program(void) const { return active_program; }
		// Updates uniform buffers for camera and lighting.
		virtual void update_uniforms(void) = 0;
		// Binds data to a uniform buffer.
		void bind_uniform(UniformBuffer buffer, GLsizeiptr size, const GLvoid * data);

		// Checks if a given extension is supported.
		inline bool is_ext_supported(const std::string& extension) const { return SDL_GL_ExtensionSupported(extension.c_str()) == SDL_TRUE; }
	};
}