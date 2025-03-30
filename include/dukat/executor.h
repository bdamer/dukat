#pragma once

#ifdef WIN32

#include <windows.h>

namespace dukat
{
	// Wraps win32 process execution logic.
	struct Executor
	{
		// Runtime variables
		HANDLE read_handle;
		HANDLE write_handle;
		PROCESS_INFORMATION pi;

		// Command results
		DWORD exit_code;
		std::string output;

		Executor(const std::string& command_line);
		~Executor(void);
	};
}

#endif
