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

/**
    Copyright 2022 Nongus Studios (https://github.com/NongusStudios/shard)
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/