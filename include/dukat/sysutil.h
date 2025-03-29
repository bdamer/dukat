#pragma once

namespace dukat
{
	inline constexpr uint32_t mc_const(char a, char b, char c, char d) 
	{
		return static_cast<uint32_t>((a << 24) | (b << 16) | (c << 8) | d);
	}

	inline uint16_t read_short(const uint8_t* ptr)
	{
		return (*(ptr + 1) << 8) | (*ptr);
	}

	inline uint32_t read_int(const uint8_t* ptr)
	{
		return (*(ptr + 3) << 24) | (*(ptr + 2) << 16) | (*(ptr + 1) << 8) | (*ptr);
	}

	inline float_t read_float(const uint8_t* ptr)
	{
		auto val = read_int(ptr);
		return reinterpret_cast<float_t&>(val);
	}

	inline std::string file_extension(const std::string& path)
	{
		const auto idx = path.rfind('.');
		if (idx != std::string::npos)
			return path.substr(idx + 1);
		else
			return "";
	}

	inline std::string file_name(const std::string& path)
	{
		const auto idx = path.find_last_of("\\/");
		if (idx != std::string::npos)
			return path.substr(idx + 1);
		else
			return path;
	}

	inline std::string dir_name(const std::string& path)
	{
		const auto idx = path.find_last_of("\\/");
		if (idx != std::string::npos)
			return path.substr(0, idx + 1);
		else
			return "";
	}

	/// <summary>
	/// Checks for and logs error of last OpenGL operation.
	/// </summary>
	void gl_check_error(void);

	/// <summary>
	/// Creates a new crash dump directory and copies any of the 
	/// provided files to it.
	/// </summary>
	/// <param name="paths">Array of file paths to include in the dump.</param>
	void crash_dump(const std::vector<std::string>& paths);

	/// <summary>
	/// Checks if file exists.
	/// </summary>
	/// <param name="path">File path</param>
	/// <returns>True if file exist, otherwise false.</returns>
	bool file_exists(const std::string& path);

	/// <summary>
	/// Checks if directory exists.
	/// </summary>
	/// <param name="path">Directory path</param>
	/// <returns>True if directory exist, otherwise false.</returns>
	bool directory_exists(const std::string& path);

	/// <summary>
	/// Creates a new directory.
	/// </summary>
	/// <param name="path">Directory path</param>
	/// <returns>True if directory was created, otherwise false.</returns>
	bool create_directory(const std::string& path);

	/// <summary>
	/// Copies a file to a new destination.
	/// </summary>
	/// <param name="source">Source path</param>
	/// <param name="dest">Destination path</param>
	/// <returns>True if file was copied, otherwise false.</returns>
	bool copy_file(const std::string& source, const std::string& dest);

	/// <summary>
	///	Moves a file to a new destination.
	/// </summary>
	/// <param name="source">Source path</param>
	/// <param name="dest">Destination path</param>
	/// <returns>True if file was moved, otherwise false.</returns>
	bool move_file(const std::string& source, const std::string& dest);

	/// <summary>
	/// Deletes a file from the filesystem.
	/// </summary>
	/// <param name="path">The path</param>
	/// <returns>True if file was deleted, otherwise false.</returns>
	bool delete_file(const std::string& path);

	/// <summary>
	/// Returns the current working directory.
	/// </summary>
	/// <returns>Working directory.</returns>
	std::string current_working_directory(void);

	/// <summary>
	/// Lists files under a given directory.
	/// </summary>
	/// <param name="path">Directory path</param>
	/// <returns>A list of file names.</returns>
	std::vector<std::string> list_files(const std::string& path);

	/// <summary>
	/// Executes a command and returns the output.
	/// </summary>
	/// <param name="command_line">The command to execute.</param>
	/// <param name="output">The output</param>
	/// <returns>The exit code of the process.</returns>
	int execute_command(const std::string& command_line, std::string& output);
}