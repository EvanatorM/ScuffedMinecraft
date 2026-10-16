#pragma once

#include <wv/pch.h>
#include <cstdio>

namespace WillowVox
{
	enum class LogLevel
	{
		Info,
		Warning,
		Error,
		Fatal,
		Success
	};

	class Logger {
	public:
		static void Init();
		static void Shutdown();

		static void Log(LogLevel level, const char* format, ...);

	private:
		static std::FILE* _logFile;
	};
}

#define LOG_INFO(...)  WillowVox::Logger::Log(WillowVox::LogLevel::Info, __VA_ARGS__)
#define LOG_WARN(...)  WillowVox::Logger::Log(WillowVox::LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...) WillowVox::Logger::Log(WillowVox::LogLevel::Error, __VA_ARGS__)
#define LOG_FATAL(...) WillowVox::Logger::Log(WillowVox::LogLevel::Fatal, __VA_ARGS__)
#define LOG_SUCCESS(...) WillowVox::Logger::Log(WillowVox::LogLevel::Success, __VA_ARGS__)