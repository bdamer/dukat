#include "stdafx.h"
#include "buffers.h"
#include "log.h"
#include "perfcounter.h"

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

	FrameBuffer::FrameBuffer(int width, int height, bool create_color_buffer, bool create_depth_buffer) 
		: width(width), height(height), fbo(0), texture(0), rbo(0)
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		if (create_color_buffer)
		{
			glGenTextures(1, &texture);
		}

		if (create_depth_buffer)
		{
			glGenRenderbuffers(1, &rbo);
		}

		rebuild();

		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	FrameBuffer::~FrameBuffer(void)
	{
		if (fbo >= 0)
		{
			glDeleteFramebuffers(1, &fbo);
			fbo = -1;
		}
		if (texture >= 0)
		{
			glDeleteTextures(1, &texture);
			texture = -1;
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
		if (texture)
		{
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
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
	}

	void FrameBuffer::resize(int width, int height)
	{
		if ((this->width == width) && (this->height == height))
			return;

		this->width = width;
		this->height = height;

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
}