#include "stdafx.h"
#include "renderer3.h"
#include "buffers.h"
#include "log.h"
#include "mesh.h"
#include "meshbuilder2.h"
#include "meshgroup.h"
#include "perfcounter.h"
#include "shadercache.h"
#include "shaderprogram.h"
#include "sysutil.h"

namespace dukat
{
	// Required definitions
	constexpr int Renderer3::fbo_size;

	Renderer3::Renderer3(Window* window, ShaderCache* shader_cache, TextureCache* textures)
		: Renderer(window, shader_cache), effects_enabled(true)
	{
		// Enable transparency
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// Enable depth buffer
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

#if OPENGL_VERSION >= 30
        // Enable primitive restart - only available in OpenGL >= 3.1 
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(primitive_restart);

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
		glBindTexture(GL_TEXTURE_2D, frame_buffer->texture);

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

	void Renderer3::render(const std::vector<Renderable*>& meshes)
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

		window->clear();

		// Scene pass
		glEnable(GL_DEPTH_TEST);
		for (auto& it : meshes)
		{
			if (it->visible && it->stage == RenderStage::SCENE)
			{
				it->render(this);
			}
		}

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
			glViewport(0, 0, window->get_width(), window->get_height());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, fb0->texture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, frame_buffer->texture);

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
			if (it->visible && it->stage == RenderStage::OVERLAY)
			{
				it->render(this);
			}
		}

        window->present();
    }

	void Renderer3::update_uniforms(void)
	{
#if OPENGL_VERSION >= 30
		// Update uniform buffers
		glBindBufferBase(GL_UNIFORM_BUFFER, UniformBuffer::CAMERA, uniform_buffers->buffers[0]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraTransform3), &camera->transform, GL_STREAM_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, UniformBuffer::LIGHT, uniform_buffers->buffers[1]);
		glBufferData(GL_UNIFORM_BUFFER, num_lights * sizeof(Light), &lights, GL_STREAM_DRAW);
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
		// TODO: bind lights
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