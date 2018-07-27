#include "stdafx.h"
#include "log.h"
#include "settings.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/file_sinks.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/sinks/stdout_sinks.h>

namespace dukat
{
	std::shared_ptr<spdlog::logger> log = spdlog::stdout_logger_st("default");

	void init_logging(const Settings& settings)
	{
		spdlog::set_pattern("[%Y-%m-%d %H-%M-%S.%e] [%l] %v");
		// Enable async flush every 2 seconds
		spdlog::set_async_mode(4096, spdlog::async_overflow_policy::block_retry,
                       nullptr,
                       std::chrono::seconds(2));

		const auto log_output = settings.get_string("logging.output", "console");
		if (log_output == "console")
		{
			log = spdlog::stdout_logger_st("console");
		}
		else if (log_output == "file")
		{
			const auto log_file = settings.get_string("logging.file", "run.log");
			log = spdlog::create<spdlog::sinks::simple_file_sink_st>("file_logger", log_file);
		}
		else
		{
			log = spdlog::create<spdlog::sinks::null_sink_st>("null_logger");
		}

		const auto log_level =  settings.get_string("logging.level", "debug");
		spdlog::level::level_enum level;
		if (log_level == "error")
		{
			level = spdlog::level::err;
		}
		else if (log_level == "warn")
		{
			level = spdlog::level::warn;
		}
		else if (log_level == "info")
		{
			level = spdlog::level::info;
		}
		else if (log_level == "debug")
		{
			level = spdlog::level::debug;
		}
		log->set_level(level);
	}
}
