#pragma once

#include <spdlog/spdlog.h>

namespace dukat
{
	class Settings;

	// Initialize logging subsystem
	void init_logging(const Settings& settings);

	// Global logger
	extern std::shared_ptr<spdlog::logger> log;
}