#pragma once

#include "rad/Core/Common.h"
#include "rad/Core/String.h"

enum class LogLevel
{
    Undefined,
    Verbose,
    Debug,
    Info,
    Warn,
    Error,
    Critical,
    Count
};

std::string ToString(LogLevel level);

void LogPrint(const char* category, LogLevel level, const char* format, ...);
