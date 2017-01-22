#include "stdafx.h"
#include "buffers.h"
#include "log.h"
#include "perfcounter.h"

namespace dukat
{
	FrameBuffer::FrameBuffer(int width, int height, bool hasDepth) : width(width), height(height), fbo(-1), texture(-1), rbo(-1)
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		// Create and attach texture
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		// TODO: make format configurable; if we do, make sure GL_UNSIGNED_BYTE matches GL_RGBA and is applied 
		// correctly in resize
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

		if (hasDepth)
		{
			glGenRenderbuffers(1, &rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
		}

		GLenum status;
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			std::stringstream ss;
			ss << "Framebuffer check failed: " << status;
			throw std::runtime_error(ss.str());
		}

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

	void FrameBuffer::resize(int width, int height)
	{
		if ((this->width == width) && (this->height == height))
			return;

		this->width = width;
		this->height = height;

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		// Resize texture
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Resize depth buffer
		if (rbo >= 0)
		{
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
			glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::bind(void)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, width, height);
	}
}