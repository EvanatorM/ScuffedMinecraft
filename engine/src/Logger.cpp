#include <wv/Logger.h>

#include <iostream>
#include <cstdarg>
#include <ctime>

namespace WillowVox
{
	#ifdef _WIN32
	#include <Windows.h>
	void SetColor(LogLevel logLevel)
	{
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		switch (logLevel)
		{
		case LogLevel::Info:
			SetConsoleTextAttribute(hConsole, (0 << 4) | 15);
			break;
		case LogLevel::Warning:
			SetConsoleTextAttribute(hConsole, (0 << 4) | 14);
			break;
		case LogLevel::Error:
			SetConsoleTextAttribute(hConsole, (0 << 4) | 12);
			break;
		case LogLevel::Fatal:
			SetConsoleTextAttribute(hConsole, (12 << 4) | 0);
			break;
		case LogLevel::Success:
			SetConsoleTextAttribute(hConsole, (0 << 4) | 2);
			break;
		}
	}
	#else
	void SetColor(LogLevel logLevel)
	{
		switch (logLevel)
		{
		case LogLevel::Info:
			printf("\x1b[0m");
			break;
		case LogLevel::Warning:
			printf("\x1b[93m");
			break;
		case LogLevel::Error:
			printf("\x1b[91m");
			break;
		case LogLevel::Fatal:
			printf("\x1b[41m");
			break;
		case LogLevel::Success:
			printf("\x1b[32m");
			break;
		}
	}
	#endif


	std::FILE* Logger::_logFile = nullptr;

	void Logger::Init()
	{
		_logFile = std::fopen("output.log", "w");

		if (!_logFile)
			std::cerr << "FATAL: Failed to open output.log!\n";
		else
			LOG_INFO("Logger initialized successfully");
	}

	void Logger::Shutdown()
	{
		if (_logFile)
		{
			LOG_INFO("Shutting down logger");
			std::fclose(_logFile);
			_logFile = nullptr;
		}
	}

	void Logger::Log(LogLevel level, const char* format, ...)
	{
		// Get current time
		std::time_t now = std::time(nullptr);
		std::tm* timeinfo = std::localtime(&now);
		char timeStr[16];
		std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeinfo);

		// Determine prefix based on severity
		const char* levelStr = "";
		switch (level)
		{
		case LogLevel::Warning: levelStr = " WARN:"; break;
		case LogLevel::Error: levelStr = " ERROR:"; break;
		case LogLevel::Fatal: levelStr = " FATAL:"; break;
		}

		// Unpack arguments into a buffer
		char messageBuffer[1024];
		va_list args;
		va_start(args, format);
		vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);
		va_end(args);

		// Print to console
		SetColor(level);
		printf("[%s]%s %s\n", timeStr, levelStr, messageBuffer);
		SetColor(LogLevel::Info);

		// Print to file
		if (_logFile)
		{
			std::fprintf(_logFile, "[%s]%s %s\n", timeStr, levelStr, messageBuffer);
			std::fflush(_logFile);
		}
	}
}