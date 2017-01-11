#pragma once

#include <GL/glew.h>
#include "perfcounter.h"

// TODO: revisit number of buffers and need for centralized manager to control resource usage
namespace dukat
{
	// Generic OpenGL buffer
	struct GenericBuffer
	{
		const int count;
		GLuint* buffers;

		GenericBuffer(int count) : count(count)
		{
			buffers = new GLuint[count];
			glGenBuffers(count, buffers);
		}

		~GenericBuffer(void)
		{
			glDeleteBuffers(count, buffers);
			delete[] buffers;
			perfc.inc(PerformanceCounter::BUFFER_FREE);
		}
	};

	// Wrapper around a OpenGL vertex array object.
	struct VertexBuffer
	{
		const int count;
		GLuint vao;
		GLuint* buffers;

		// Creates VAO with a specified number of buffers.
		VertexBuffer(int count) : count(count)
		{
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
			buffers = new GLuint[count];
			glGenBuffers(count, buffers);
		}

		~VertexBuffer(void)
		{
			glDeleteBuffers(count, buffers);
			glDeleteVertexArrays(1, &vao);
			delete[] buffers;
			perfc.inc(PerformanceCounter::BUFFER_FREE);
		}
	};

	struct FrameBuffer
	{
		// Framebuffer
		GLuint fbo;
		// Texture
		GLuint texture;
		// Render buffer
		GLuint rbo;
		// Dimensions
		int width, height;

		FrameBuffer(int width, int height, bool hasDepth);
		~FrameBuffer(void);

		void resize(int width, int height);
		void bind(void);
	};
}