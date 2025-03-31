#include "stdafx.h"
#include <dukat/executor.h>
#include <dukat/log.h>

namespace dukat
{
#ifdef WIN32
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

	static int win32_check_error(void)
	{
		const auto error = ::GetLastError();
		if (error)
		{
			LPTSTR msg_buf = nullptr;
			const auto length = FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				error,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&msg_buf,
				0, NULL);

			std::vector<char> chars(length + 1);
			size_t converted = 0;
			auto res = wcstombs_s(&converted, chars.data(), chars.size(), msg_buf, length);
			if (res == 0)
				log->error("win32 error: {}", chars.data());
			else
				log->error("Unable to determine error: {}", error);

			LocalFree(msg_buf);
		}
		return static_cast<int>(error);
	}

	Executor::Executor(const std::string& command_line) : read_handle(NULL), write_handle(NULL), exit_code(0), output("")
	{
		pi = { 0 };

		SECURITY_ATTRIBUTES sa = { 0 };
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;

		if (!CreatePipe(&read_handle, &write_handle, &sa, 0))
		{
			log->error("Failed to create pipe.");
			exit_code = win32_check_error();
			return;
		}

		STARTUPINFOA si = { 0 };
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.wShowWindow = SW_HIDE;
		si.hStdOutput = write_handle;
		si.hStdError = write_handle;
		si.hStdInput = NULL;

		auto cmd_ptr = const_cast<char*>(command_line.c_str());
		const auto result = CreateProcessA(NULL, cmd_ptr,
			NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
		if (result == 0)
		{
			log->error("Failed to execute command: {}", command_line);
			exit_code = win32_check_error();
			return;
		}

		CloseHandle(write_handle);
		write_handle = NULL;

		const auto buffer_size = 4096;
		char buffer[buffer_size];
		DWORD bytes_read;
		while (ReadFile(read_handle, buffer, buffer_size - 1, &bytes_read, NULL) && bytes_read != 0)
		{
			buffer[bytes_read] = '\0';
			output += buffer;
		}

		log->debug("Waiting for process to finish...");
		WaitForSingleObject(pi.hProcess, INFINITE);
		GetExitCodeProcess(pi.hProcess, &exit_code);
	}

	Executor::~Executor(void)
	{
		if (write_handle)
			CloseHandle(write_handle);
		if (read_handle)
			CloseHandle(read_handle);
		if (pi.hThread)
			CloseHandle(pi.hThread);
		if (pi.hProcess)
			CloseHandle(pi.hProcess);
	}

#endif

	int execute_command(const std::string& command_line, std::string& output)
	{
		log->debug("Executing: {}", command_line);
#ifdef WIN32
		Executor exec(command_line);
		output = exec.output;
		const auto res = static_cast<int>(exec.exit_code);
#else
		auto pipe = popen(command_line.c_str(), "r");
		if (!pipe)
		{
			log->error("Failed to execute command: {}", command_line);
			return -1;
		}

		char buffer[128];
		while (!feof(pipe))
		{
			if (fgets(buffer, 128, pipe) != NULL)
				output += buffer;
		}
		const auto res = pclose(pipe);
#endif

		log->debug("Process returned: {}", res);
		return res;
	}
}
