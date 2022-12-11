#pragma once

#include "rad/Core/Common.h"

#include <atomic>

#include <condition_variable>
#include <future>
#include <mutex>
#include <shared_mutex>
#include <thread>

// C++20
#include <barrier>
#include <latch>
#include <semaphore>
#include <stop_token>

#include <functional>
#include "rad/Core/Exception.h"

#ifdef _WIN32
#include <sdkddkver.h>
#endif

// Fixed-size thread pool: https://www.boost.org/doc/libs/1_80_0/doc/html/boost_asio/reference/thread_pool.html
#include "boost/asio/thread_pool.hpp"
