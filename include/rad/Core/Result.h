#pragma once

#include "rad/Core/Common.h"

enum class Result
{
    Success = 0,
    NotReady = 1,
    Timeout = 2,

    Error = -1,
};
