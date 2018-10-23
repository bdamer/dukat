#pragma once

#ifdef __ANDROID__
#include <android/log.h>
#define LOG_TAG "DUKAT"
#else
#include <spdlog/spdlog.h>
#endif

namespace dukat
{
	class Settings;

	// Initialize logging subsystem
	void init_logging(const Settings& settings);

#ifndef __ANDROID__
	// Global logger
	extern std::shared_ptr<spdlog::logger> log;
#else

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"
	struct Logger {

        void error(const char *fmt) 
		{ 
			__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt);
		}

        void warn(const char *fmt) 
		{ 
			__android_log_print(ANDROID_LOG_WARN, LOG_TAG, fmt);
		}
		
		void info(const char *fmt) 
		{ 
			__android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt);
		}
		
		void debug(const char *fmt) 
		{ 
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt);
		}

        void trace(const char *fmt) 
		{ 
			__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, fmt);
		}

		template<typename Arg1, typename... Args>
		void info(const char *fmt, const Arg1 &, const Args &... args) { };
		template<typename Arg1, typename... Args>
		void error(const char *fmt, const Arg1 &, const Args &... args) { };
		template<typename Arg1, typename... Args>
		void warn(const char *fmt, const Arg1 &, const Args &... args) { };
		template<typename Arg1, typename... Args>
		void debug(const char *fmt, const Arg1 &, const Args &... args) { };
        template<typename Arg1, typename... Args>
        void trace(const char *fmt, const Arg1 &, const Args &... args) { };
	};

	extern std::shared_ptr<Logger> log;
#pragma clang diagnostic pop

#endif

}