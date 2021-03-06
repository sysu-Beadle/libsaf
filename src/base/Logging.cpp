//
// Created by beadle on 3/17/17.
//

#include <algorithm>
#include <iostream>
#include <chrono>
#include <stdarg.h>

#include "Logging.h"
#include "AsyncLogger.h"


namespace saf
{
	typedef FixedBuffer<128*1024> LoggerBuffer;

	static AsyncLogger gLogger("log/game.log");

	namespace
	{
		const char* LogLevelName[int(LogLevel::COUNT)] =
		{
			"DEBUG -",
			"INFO -",
			"WARN -",
			"ERROR -",
			"FATAL -",
		};

		LogLevel gLogLevel = LogLevel::DEBUG;

		const size_t kMaxNumericSize = 32;
		const char digits[] = "0123456789";
		__thread char errorBuffer[512];

		// Efficient Integer to String Conversions, by Matthew Wilson.
		template<typename T>
		size_t convert(char buf[], T value)
		{
			T i = value;
			char* p = buf;
			do
			{
				int lsd = static_cast<int>(i % 10);
				i /= 10;
				*p++ = digits[lsd];
			} while (i != 0);

			if (value < 0)
			{
				*p++ = '-';
			}
			*p = '\0';
			std::reverse(buf, p);
			return p - buf;
		}

		template <typename T>
		void appendInteger(LoggerBuffer& buffer, T v)
		{
			if (buffer.avail() >= kMaxNumericSize)
			{
				size_t len = convert(buffer.current(), v);
				buffer.add(len);
			}
		}

		void appendString(LoggerBuffer& buffer, const char* str)
		{
			buffer.append(str, strlen(str));
		}

		void appendString(LoggerBuffer& buffer, const char* str, size_t len)
		{
			buffer.append(str, len);
		}

		void appendTime(LoggerBuffer& buffer)
		{
			std::chrono::high_resolution_clock::time_point p = std::chrono::high_resolution_clock::now();
			std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(p.time_since_epoch());
			std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds>(ms);

			std::time_t t = s.count();
			int fractional_seconds = ms.count() % 1000;

			std::string timeStr = std::ctime(&t);
			auto endPos = timeStr.find_last_of(' ');
			auto starPos = timeStr.rfind(' ', endPos - 1);
			buffer.append(timeStr.c_str() + starPos, endPos - starPos);
			buffer.append(":", 1);
			appendInteger(buffer, fractional_seconds);
			buffer.append(" ", 1);
		}

	}

	void log(LogLevel level, const char* file, int line, const char* function, int error, const char* format, ...)
	{
		if (level < gLogLevel)
			return;

		static __thread FixedBuffer<128*1024> buffer;
		static __thread char content[1024] = "";
		buffer.reset();

		appendString(buffer, LogLevelName[int(level)]);

		if (error != 0)
		{
			appendString(buffer, " ", 1);
			appendString(buffer, errnoToString(error));
			appendString(buffer, " errno(", 7);
			appendInteger(buffer, error);
			appendString(buffer, ") -", 3);
		}

		appendTime(buffer);
		appendString(buffer, "- ", 2);
		appendString(buffer, file);
		appendString(buffer, "(", 1);
		appendInteger(buffer, line);
		appendString(buffer, ")", 1);
		if (function)
		{
			appendString(buffer, " - ", 3);
			appendString(buffer, function);
		}
		appendString(buffer, ": ", 2);

		va_list args;
		va_start(args, format);
		vsprintf(content, format, args);
		va_end(args);
		appendString(buffer, content);

		appendString(buffer, "\n", 1);
		gLogger.append(buffer.data(), buffer.length());

		if (level >= LogLevel::FATAL)
			::abort();
	}

	void setLogLevel(LogLevel level)
	{
		gLogLevel = level;
	}

	const char* errnoToString(int error)
	{
		return strerror_r(error, errorBuffer, sizeof errorBuffer);
	}

	LoggerLauncher::LoggerLauncher(const char* path, LogLevel level)
	{
		setLogLevel(level);
		gLogger.setPath(path);
		gLogger.start();
	}

	LoggerLauncher::~LoggerLauncher()
	{
		gLogger.stop();
	}
}