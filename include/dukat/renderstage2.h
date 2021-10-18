#pragma once

#include <string>

#include "renderlayer2.h"

namespace dukat
{
	// A render stage represents one or more layers rendering 
	// to the same screen buffer.
	struct RenderStage2
	{
		const int id;
		// list of layers ordered by priority
		std::list<std::unique_ptr<RenderLayer2>> layers;

		// composite program to render to intermediate target
		// before compositing to screen buffer.
		ShaderProgram* composite_program;
		std::function<void(ShaderProgram*)> composite_binder;
		std::unique_ptr<FrameBuffer> frame_buffer;

		RenderStage2(int id) : id(id), composite_program(nullptr), composite_binder(nullptr), frame_buffer(nullptr) { }
		~RenderStage2(void) { }
	};
}