#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <ctime>
#include <fstream>
#include <cstdio>
#include <iomanip>

#define LOG_COLOR_RESET        "\x1b[0m"
#define LOG_COLOR_APP_WARN     "\x1b[93m"
#define LOG_COLOR_APP_ERROR    "\x1b[91m"
#define LOG_COLOR_ENGINE_LOG   "\x1b[32m"
#define LOG_COLOR_ENGINE_WARN  "\x1b[33m"
#define LOG_COLOR_ENGINE_ERROR "\x1b[41m"

namespace WillowVox
{
	class WILLOWVOX_API Logger
	{
	public:
		// Game
		template <typename... Args>
		static void Log(const char* msg, Args... args)
		{
			std::time_t t = std::time(nullptr);
			std::tm now;
			localtime_r(&t, &now);
			printf(LOG_COLOR_RESET "[%d:%d:%d App] ", now.tm_hour, now.tm_min, now.tm_sec);
			printf(msg, args...);
			printf("\n");

			char formattedMsg[1024];
			std::snprintf(formattedMsg, sizeof(formattedMsg), msg, args...);
			std::fstream logFile("log.txt");
			logFile << "[" << std::setfill('0') << std::setw(2) << now.tm_hour << ":"
				<< std::setfill('0') << std::setw(2) << now.tm_min << ":"
				<< std::setfill('0') << std::setw(2) << now.tm_sec << " App] "
				<< formattedMsg << std::endl;
			logFile.close();
		}

		template <typename... Args>
		static void Warn(const char* msg, Args... args)
		{
			std::time_t t = std::time(nullptr);
			std::tm now;
			localtime_r(&t, &now);
			printf(LOG_COLOR_APP_WARN "[%d:%d:%d App] WARN: ", now.tm_hour, now.tm_min, now.tm_sec);
			printf(msg, args...);
			printf(LOG_COLOR_RESET "\n");

			char formattedMsg[1024];
			std::snprintf(formattedMsg, sizeof(formattedMsg), msg, args...);
			std::fstream logFile("log.txt");
			logFile << "[" << std::setfill('0') << std::setw(2) << now.tm_hour << ":"
				<< std::setfill('0') << std::setw(2) << now.tm_min << ":"
				<< std::setfill('0') << std::setw(2) << now.tm_sec << " App] WARN: "
				<< formattedMsg << std::endl;
			logFile.close();
		}

		template <typename... Args>
		static void Error(const char* msg, Args... args)
		{
			std::time_t t = std::time(nullptr);
			std::tm now;
			localtime_r(&t, &now);
			printf(LOG_COLOR_APP_ERROR "[%d:%d:%d App] ERROR: ", now.tm_hour, now.tm_min, now.tm_sec);
			printf(msg, args...);
			printf(LOG_COLOR_RESET "\n");

			char formattedMsg[1024];
			std::snprintf(formattedMsg, sizeof(formattedMsg), msg, args...);
			std::fstream logFile("log.txt");
			logFile << "[" << std::setfill('0') << std::setw(2) << now.tm_hour << ":"
				<< std::setfill('0') << std::setw(2) << now.tm_min << ":"
				<< std::setfill('0') << std::setw(2) << now.tm_sec << " App] ERROR: "
				<< formattedMsg << std::endl;
			logFile.close();
		}

		// Engine
		template <typename... Args>
		static void EngineLog(const char* msg, Args... args)
		{
			std::time_t t = std::time(nullptr);
			std::tm now;
			localtime_r(&t, &now);
			printf(LOG_COLOR_ENGINE_LOG "[%d:%d:%d Engine] ", now.tm_hour, now.tm_min, now.tm_sec);
			printf(msg, args...);
			printf(LOG_COLOR_RESET "\n");

			char formattedMsg[1024];
			std::snprintf(formattedMsg, sizeof(formattedMsg), msg, args...);
			std::fstream logFile("log.txt");
			logFile << "[" << std::setfill('0') << std::setw(2) << now.tm_hour << ":"
				<< std::setfill('0') << std::setw(2) << now.tm_min << ":"
				<< std::setfill('0') << std::setw(2) << now.tm_sec << " Engine] "
				<< formattedMsg << std::endl;
			logFile.close();
		}

		template <typename... Args>
		static void EngineWarn(const char* msg, Args... args)
		{
			std::time_t t = std::time(nullptr);
			std::tm now;
			localtime_r(&t, &now);
			printf(LOG_COLOR_ENGINE_WARN "[%d:%d:%d Engine] WARN: ", now.tm_hour, now.tm_min, now.tm_sec);
			printf(msg, args...);
			printf(LOG_COLOR_RESET "\n");

			char formattedMsg[1024];
			std::snprintf(formattedMsg, sizeof(formattedMsg), msg, args...);
			std::fstream logFile("log.txt");
			logFile << "[" << std::setfill('0') << std::setw(2) << now.tm_hour << ":"
				<< std::setfill('0') << std::setw(2) << now.tm_min << ":"
				<< std::setfill('0') << std::setw(2) << now.tm_sec << " Engine] WARN: "
				<< formattedMsg << std::endl;
			logFile.close();
		}

		template <typename... Args>
		static void EngineError(const char* msg, Args... args)
		{
			std::time_t t = std::time(nullptr);
			std::tm now;
			localtime_r(&t, &now);
			printf(LOG_COLOR_ENGINE_ERROR "[%d:%d:%d Engine] ERROR: ", now.tm_hour, now.tm_min, now.tm_sec);
			printf(msg, args...);
			printf(LOG_COLOR_RESET "\n");

			char formattedMsg[1024];
			std::snprintf(formattedMsg, sizeof(formattedMsg), msg, args...);
			std::fstream logFile("log.txt");
			logFile << "[" << std::setfill('0') << std::setw(2) << now.tm_hour << ":"
				<< std::setfill('0') << std::setw(2) << now.tm_min << ":"
				<< std::setfill('0') << std::setw(2) << now.tm_sec << " Engine] ERROR: "
				<< formattedMsg << std::endl;
			logFile.close();
		}
	};
}