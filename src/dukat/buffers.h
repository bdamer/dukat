#pragma once

#include <GL/glew.h>
#include "dukat.h"
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

	// Wrapper around a OpenGL vertex array object in OpenGL 3 and higher.
	// In compatibility mode, will use vertex buffer object.
	struct VertexBuffer
	{
		const int buffer_count;
#if OPENGL_VERSION >= 30
		GLuint vao; // Vertex Array Object
#endif
		GLuint* buffers; // Array of buffers
		GLint* counts; // Array of element count for each buffer
		GLsizei* strides; // Array of size of individual elements for each buffer

		// Creates VAO with a specified number of buffers.
		VertexBuffer(int buffer_count) : buffer_count(buffer_count)
		{
#if OPENGL_VERSION >= 30
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
#endif
			buffers = new GLuint[buffer_count];
			glGenBuffers(buffer_count, buffers);
			counts = new GLint[buffer_count];
			strides = new GLsizei[buffer_count];
		}

		~VertexBuffer(void)
		{
			glDeleteBuffers(buffer_count, buffers);
#if OPENGL_VERSION >= 30
			glDeleteVertexArrays(1, &vao);
#endif
			delete[] buffers;
			delete[] counts;
			delete[] strides;
			perfc.inc(PerformanceCounter::BUFFER_FREE);
		}

		void load_data(int buffer_index, GLenum target, int count, GLsizei stride, const GLvoid* data, GLenum usage);
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
		// Stores last viewport for unbind
		GLint last_viewport[4];

		FrameBuffer(int width, int height, bool create_draw_buffer, bool create_depth_buffer);
		~FrameBuffer(void);

		void rebuild(void);
		void resize(int width, int height);
		void bind(void);
		void unbind(void);
	};
}