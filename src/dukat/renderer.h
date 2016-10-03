#pragma once

#include "window.h"

namespace dukat
{
	class ShaderCache;
	class ShaderProgram;
	struct GenericBuffer;

	// Base class for 2D and 3D renderers.
	class Renderer : public WindowEventListener
	{
	protected:		
		Window* window;
		ShaderCache* shader_cache;
		// The currently active shader program
		ShaderProgram* active_program;
		// Uniform buffers
		std::unique_ptr<GenericBuffer> uniform_buffers;
		bool use_wireframe;

		void test_capabilities(void);

	public:
		enum UniformBuffer
		{
			CAMERA,
			LIGHT,
			RESERVED1,
			RESERVED2,
			RESERVED3,
			MODEL,
			MATERIAL,
			_COUNT
		};

		// Vertex attributes
		static constexpr const char* at_pos = "a_position";
		static constexpr const char* at_normal = "a_normal";
		static constexpr const char* at_color = "a_color";
		static constexpr const char* at_texcoord = "a_texCoord";
		// Uniform variables
		static constexpr const char* uf_color = "u_color";
		static constexpr const char* uf_model = "u_model";
		// TODO: use a single array
		static constexpr const char* uf_tex0 = "u_tex0";
		static constexpr const char* uf_tex1 = "u_tex1";
		static constexpr const char* uf_tex2 = "u_tex2";
		static constexpr const char* uf_tex3 = "u_tex3";
		// Uniform blocks
		static constexpr const char* uf_camera = "Camera";
		static constexpr const char* uf_light = "Light";
		static constexpr const char* uf_material = "Material";

		Renderer(Window* window, ShaderCache* shaders);
		virtual ~Renderer(void);

		// Called on window resize
		virtual void resize(int width, int height);

		// Display settings
		void toggle_wireframe(void);

		// Changes active program.
		void switch_shader(ShaderProgram* program);
		// Returns the currently active program.
		ShaderProgram* get_active_program(void) const { return active_program; }
		// Updates uniform buffers for camera and lighting.
		virtual void update_uniforms(void) = 0;
	};
}