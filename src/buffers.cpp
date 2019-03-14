#include "stdafx.h"
#include <dukat/buffers.h>
#include <dukat/log.h>
#include <dukat/perfcounter.h>
#include <dukat/sysutil.h>

// Rebind EXT_framebuffer_object methods
#if defined(OPENGL_CORE) && (OPENGL_CORE < 30)

#undef glGenFramebuffers
#undef glGenRenderbuffers
#undef glBindFramebuffer
#undef glBindRenderbuffer
#undef glFramebufferTexture2D
#undef glRenderbufferStorage
#undef glFramebufferRenderbuffer
#undef glCheckFramebufferStatus
#undef glDeleteFramebuffers
#undef glDeleteRenderbuffers

#define glGenFramebuffers glGenFramebuffersEXT
#define glGenRenderbuffers glGenRenderbuffersEXT
#define glBindFramebuffer glBindFramebufferEXT
#define glBindRenderbuffer glBindRenderbufferEXT
#define glFramebufferTexture2D glFramebufferTexture2DEXT
#define glRenderbufferStorage glRenderbufferStorageEXT
#define glFramebufferRenderbuffer glFramebufferRenderbufferEXT
#define glCheckFramebufferStatus glCheckFramebufferStatusEXT
#define glDeleteFramebuffers glDeleteFramebuffersEXT
#define glDeleteRenderbuffers glDeleteRenderbuffersEXT

#endif

namespace dukat
{
	void VertexBuffer::load_data(int index, GLenum target, int count, GLsizei stride, const GLvoid* data, GLenum usage)
	{
		counts[index] = data == nullptr ? 0 : count;
		strides[index] = stride;
		glBindBuffer(target, buffers[index]);
		glBufferData(target, count * stride, data, usage);
		glBindBuffer(target, 0);
	}

	FrameBuffer::FrameBuffer(int width, int height, bool create_color_buffer, bool create_depth_buffer, TextureFilterProfile profile)
		: fbo(0), texture(nullptr), rbo(0), width(width), height(height), profile(profile)
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		if (create_color_buffer)
		{
			texture = std::make_unique<Texture>(width, height);
		}

		if (create_depth_buffer)
		{
			glGenRenderbuffers(1, &rbo);
		}

		rebuild();

		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
#ifdef _DEBUG
		gl_check_error();
#endif
	}

	FrameBuffer::~FrameBuffer(void)
	{
		if (fbo >= 0)
		{
			glDeleteFramebuffers(1, &fbo);
			fbo = -1;
		}
		if (rbo >= 0)
		{
			glDeleteRenderbuffers(1, &rbo);
			rbo = -1;
		}
		perfc.inc(PerformanceCounter::BUFFER_FREE);
	}

	void FrameBuffer::rebuild(void)
	{
		if (texture != nullptr)
		{
			initialize_draw_buffer(texture.get());
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture->target, texture->id, 0);
		}

		if (rbo)
		{
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
			glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
		}

		if (texture || rbo)
		{
			auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE)
			{
				std::stringstream ss;
				ss << "Framebuffer check failed: " << status;
				throw std::runtime_error(ss.str());
			}
		}
#ifdef _DEBUG
		gl_check_error();
#endif
	}

	void FrameBuffer::resize(int width, int height)
	{
		if ((this->width == width) && (this->height == height))
			return;

		this->width = width;
		this->height = height;

		if (texture != nullptr)
		{
			texture->w = width;
			texture->h = height;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		rebuild();

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::bind(void)
	{
		glGetIntegerv(GL_VIEWPORT, last_viewport);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, width, height);
	}

	void FrameBuffer::unbind(void)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
	}

	void FrameBuffer::initialize_draw_buffer(Texture* t)
	{
		glBindTexture(t->target, t->id);
		glTexImage2D(t->target, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameterf(t->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(t->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		switch (profile)
		{
		case TextureFilterProfile::ProfileNearest:
			glTexParameteri(t->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(t->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		case TextureFilterProfile::ProfileLinear:
		default:
			glTexParameteri(t->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(t->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		}
	}

	void FrameBuffer::attach_draw_buffer(Texture* t)
	{
		glBindTexture(t->target, t->id);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t->target, t->id, 0);
	}

	void FrameBuffer::detach_draw_buffer(void)
	{
		if (texture != nullptr)
			attach_draw_buffer(texture.get());
	}
}