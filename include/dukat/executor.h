#pragma once

namespace dukat
{
	/// <summary>
	/// Executes a command and returns the output.
	/// </summary>
	/// <param name="command_line">The command to execute.</param>
	/// <param name="output">The output</param>
	/// <returns>The exit code of the process.</returns>
	int execute_command(const std::string& command_line, std::string& output);
}
