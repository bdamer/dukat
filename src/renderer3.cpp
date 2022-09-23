#include "stdafx.h"
#include <dukat/renderer3.h>
#include <dukat/buffers.h>
#include <dukat/log.h>
#include <dukat/meshdata.h>
#include <dukat/meshbuilder2.h>
#include <dukat/meshgroup.h>
#include <dukat/perfcounter.h>
#include <dukat/shadercache.h>
#include <dukat/shaderprogram.h>
#include <dukat/sysutil.h>

namespace dukat
{
	// Required definitions
	constexpr int Renderer3::fbo_size;

	Renderer3::Renderer3(Window* window, ShaderCache* shader_cache, TextureCache* textures)
		: Renderer(window, shader_cache), effects_enabled(false)
	{
		// Enable transparency
		set_blending(true);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// Enable depth buffer
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

#if OPENGL_CORE >= 31
		// Enable primitive restart - only available in OpenGL >= 3.1
		glEnable(GL_PRIMITIVE_RESTART);
		glPrimitiveRestartIndex(primitive_restart);
#elif OPENGL_ES >= 30
		// Enable primitive restart using -1 as fixed index. 
		// More specifically, the restart index will be:
		// GL_UNSIGNED_BYTE (2^8-1), GL_UNSIGNED_SHORT (2^16-1), GL_UNSIGNED_INT (2^32-1)
		glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
#endif

#if OPENGL_VERSION >= 30
		fb0 = std::make_unique<FrameBuffer>(window->get_width(), window->get_height(), true, true);
		fb1 = std::make_unique<FrameBuffer>(fbo_size, fbo_size, true, false);
		fb2 = std::make_unique<FrameBuffer>(fbo_size, fbo_size, true, false);
		MeshBuilder2 builder;
		quad = builder.build_textured_quad();
		composite_program = shader_cache->get_program("fx_default.vsh", "fx_composite.fsh");
#endif

		gl_check_error();
		init_lights();
	}

	void Renderer3::init_lights(void)
	{
		for (int i = 0; i < num_lights; i++)
		{
			lights[i].position = Vector3{ 0.0f, 0.0f, 0.0f };
			lights[i].ambient = Color{ 0.0f, 0.0f, 0.0f, 0.0f };
			lights[i].diffuse = Color{ 0.0f, 0.0f, 0.0f, 0.0f };
			lights[i].specular = Color{ 0.0f, 0.0f, 0.0f, 0.0f };
			lights[i].k0 = 1.0f;
			lights[i].k1 = 0.0f;
			lights[i].k2 = 0.0f;
		}
	}

	void Renderer3::switch_fbo(void)
	{
		// set input texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, frame_buffer->texture->id);

		if (frame_buffer == fb1.get())
		{
			fb2->bind();
			frame_buffer = fb2.get();
		}
		else
		{
			fb1->bind();
			frame_buffer = fb1.get();
		}

		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Renderer3::resize_window(int w, int h)
	{
		Renderer::resize_window(w, h);
		if (camera)
			camera->resize(w, h);
	}

	void Renderer3::render(const std::vector<Mesh*>& meshes)
	{
#if OPENGL_VERSION >= 30
		// Update uniform buffers once per frame.
		update_uniforms();

		if (effects_enabled)
		{
			fb0->bind();
			frame_buffer = fb0.get();
		}
#endif

		clear();
#ifdef OPENGL_CORE
		if (show_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
		// Scene pass
		glEnable(GL_DEPTH_TEST);

		for (auto& it : meshes)
		{
			if (it->visible && it->stage == RenderStage::Scene)
			{
				it->render(this);
			}
		}

#ifdef OPENGL_CORE
		if (show_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

		// Effects pass		
		glDisable(GL_DEPTH_TEST);

#if OPENGL_VERSION >= 30
		if (effects_enabled)
		{
			// TODO: review how useful this is - effects passes are currently using fixed
			// size texture 
			// Effects passes
			for (auto it = effects.begin(); it != effects.end(); ++it)
			{
				switch_fbo();
				switch_shader(it->program);
				glUniform1i(it->program->attr(Renderer::uf_tex0), 0);
				for (auto it2 : it->parameters)
				{
					switch (it2.second.count)
					{
					case 1:
						glUniform1fv(it->program->attr(it2.first), 1, it2.second.values);
						break;
					case 2:
						glUniform2fv(it->program->attr(it2.first), 1, it2.second.values);
						break;
					case 3:
						glUniform3fv(it->program->attr(it2.first), 1, it2.second.values);
						break;
					case 4:
						glUniform4fv(it->program->attr(it2.first), 1, it2.second.values);
						break;
					}
				}
				quad->render(it->program);
			}

			// Composite pass
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			reset_viewport();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, fb0->texture->id);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, frame_buffer->texture->id);

			switch_shader(composite_program);
			glUniform1i(composite_program->attr(Renderer::uf_tex0), 0);
			glUniform1i(composite_program->attr(Renderer::uf_tex1), 1);
			glUniform1f(composite_program->attr("u_scale"), 0.0f);

			quad->render(composite_program);

			// reset texture units
			glActiveTexture(GL_TEXTURE0);
		}
#endif

		// Overlay pass
		for (auto& it : meshes)
		{
			if (it->visible && it->stage == RenderStage::Overlay)
			{
				it->render(this);
			}
		}

        window->present();

#if OPENGL_VERSION < 30
		// invalidate active program to force uniforms rebind during
		// next frame
		active_program = 0;
#endif
	}

	void Renderer3::update_uniforms(void)
	{
#if OPENGL_VERSION >= 30
		// Update uniform buffers
		glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(UniformBuffer::Camera), uniform_buffers->buffers[static_cast<int>(UniformBuffer::Camera)]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraTransform3), &camera->transform, GL_STREAM_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(UniformBuffer::Light), uniform_buffers->buffers[static_cast<int>(UniformBuffer::Light)]);
		glBufferData(GL_UNIFORM_BUFFER, num_lights * sizeof(Light3), &lights, GL_STREAM_DRAW);
#else
		// Update individual uniforms.
		glUniformMatrix4fv(active_program->attr(Renderer3::u_cam_proj_pers), 1, false, camera->transform.mat_proj_pers.m);
		glUniformMatrix4fv(active_program->attr(Renderer3::u_cam_proj_orth), 1, false, camera->transform.mat_proj_orth.m);
		glUniformMatrix4fv(active_program->attr(Renderer3::u_cam_view), 1, false, camera->transform.mat_view.m);
		glUniformMatrix4fv(active_program->attr(Renderer3::u_cam_view_inv), 1, false, camera->transform.mat_view.m);
		glUniform4fv(active_program->attr(Renderer3::u_cam_position), 1, (GLfloat*)(&camera->transform.position));
		glUniform4fv(active_program->attr(Renderer3::u_cam_dir), 1, (GLfloat*)(&camera->transform.dir));
		glUniform4fv(active_program->attr(Renderer3::u_cam_up), 1, (GLfloat*)(&camera->transform.up));
		glUniform4fv(active_program->attr(Renderer3::u_cam_left), 1, (GLfloat*)(&camera->transform.right));
		// Bind light information
		for (auto i = 0; i < num_lights; i++)
		{
			const auto& light = lights[i];
			const auto light_index = GL_LIGHT0 + i;
			glLightfv(light_index, GL_POSITION, (GLfloat*)(&light.position));
			glLightfv(light_index, GL_AMBIENT, (GLfloat*)(&light.ambient));
			glLightfv(light_index, GL_DIFFUSE, (GLfloat*)(&light.diffuse));
			glLightfv(light_index, GL_SPECULAR, (GLfloat*)(&light.specular));
			glLightf(light_index, GL_CONSTANT_ATTENUATION, light.k0);
			glLightf(light_index, GL_LINEAR_ATTENUATION, light.k1);
			glLightf(light_index, GL_QUADRATIC_ATTENUATION, light.k2);
		}
#endif
	}

	void Renderer3::add_effect(int index, const Effect3& effect)
	{
		int i = 0;
		for (auto it = effects.begin(); it != effects.end(); ++it, i++)
		{
			if (i == index)
			{
				effects.insert(it, effect);
				return;
			}
		}
		effects.push_back(effect);
	}
}