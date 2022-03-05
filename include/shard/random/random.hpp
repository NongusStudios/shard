#pragma once

#include <random>

namespace shard{
    namespace randy{
        class Random{
            public:
                Random(const size_t seed);

                void setSeed(const size_t seed){
                    rng.seed(seed);
                }

                // Produces random uint32_t between 0 and UINT32_MAX
                uint32_t randi();
                // Produces random float between 0.0f and 1.0f
                float    randf();
                // Produces random double between 0.0 and 1.0
                double   randf64();

                float   randRangef  (const float   min, const float   max);
                double  randRangef64(const double  min, const double  max);
                int32_t randRange   (const int32_t min, const int32_t max);
                int64_t randRange64 (const int64_t min, const int64_t max);
            private:
                std::mt19937 rng;
        };
    } // namespace randy
} // namespace shard

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