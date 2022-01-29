#pragma once

#include <iostream>

#if defined(__GNUC__)
    #define SHARD_FUNC __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
    #define SHARD_FUNC __FUNCSIG__
#elif defined(__MINGW32__) || defined(__MINGW64__)
    #define SHARD_FUNC __PRETTY_FUNCTION__
#else
    #define SHARD_FUNC __func__
#endif

namespace shard{
    #ifdef NDEBUG
        static inline const bool IS_DEBUG = false;
    #else
        static inline const bool IS_DEBUG = true;
    #endif
}