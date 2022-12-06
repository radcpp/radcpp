#include "rad/Core/Logging.h"
#include "rad/Core/File.h"

#include <cstdarg>

#include <chrono>
#include <mutex>

static File g_logFile;

const char* g_logLevelStrings[ToUnderlying(LogLevel::Count)] =
{
    "Undefined",
    "Verbose",
    "Debug",
    "Info",
    "Warn",
    "Error",
    "Critical",
};

std::string ToString(LogLevel level)
{
    return g_logLevelStrings[ToUnderlying(level)];
}

void LogOutput(const char* data, size_t sizeInBytes);

void LogPrint(const char* category, LogLevel level, const char* format, ...)
{
    using Clock = std::chrono::system_clock;
    Clock::time_point timestamp = std::chrono::system_clock::now();

    thread_local std::string message;
    va_list args;
    va_start(args, format);
    StrPrintInPlaceArgList(message, format, args);
    va_end(args);

    long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()).count() % 1000;
    std::time_t timepoint = std::chrono::system_clock::to_time_t(timestamp);
    std::tm datetime = {};
    localtime_s(&datetime, &timepoint);

    thread_local std::string buffer;
    int charsPrinted = StrPrintInPlace(buffer, "[%02d:%02d:%02d.%03lld] %s: %s: %s\n",
        datetime.tm_hour, datetime.tm_min, datetime.tm_sec, milliseconds,
        category, g_logLevelStrings[ToUnderlying(level)], message.data());

    if (charsPrinted > 0)
    {
        LogOutput(buffer.data(), charsPrinted);

        if (level >= LogLevel::Warn)
        {
            g_logFile.Flush();
        }
    }
}

static std::mutex g_logOutputMutex;

void LogOutput(const char* data, size_t sizeInBytes)
{
    std::lock_guard lockGuard(g_logOutputMutex);

    if (!g_logFile.IsOpen())
    {
        std::string processName = io::GetProcessName();
        g_logFile.Open(processName + ".log", FileOpenWrite);
    }

    if (g_logFile.IsOpen())
    {
        g_logFile.Write(data, sizeInBytes);
    }

    fprintf(stderr, "%s", data);
}
