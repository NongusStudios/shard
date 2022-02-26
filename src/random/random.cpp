#include <shard/random/random.hpp>

#include <cassert>

namespace shard{
    namespace randy{
        Random::Random(const size_t& seed):
            rng{seed}
        {}

        uint32_t Random::randi(){
            return uint32_t(rng());
        }
        float   Random::randf(){
            return float(randi())/float(UINT32_MAX);
        }
        double  Random::randf64(){
            return double(randi())/double(UINT32_MAX);
        }

        float   Random::randRangef  (const float&   min, const float&   max){
            assert(min < max);
            float r = randf();
            float range = max - min + 1.0f;
            return r * range + min;
        }
        double  Random::randRangef64(const double&  min, const double&  max){
            assert(min < max);
            double r = randf64();
            double range = max - min + 1.0;
            return r * range + min;
        }
        int32_t Random::randRange   (const int32_t& min, const int32_t& max){
            assert(min < max);
            int32_t r = int32_t(randi() % uint32_t(INT32_MAX));
            int32_t range = max - min + 1;
            return r % range + min;
        }
        int64_t Random::randRange64 (const int64_t& min, const int64_t& max){
            assert(min < max);
            int64_t r = int64_t(randi());
            int64_t range = max - min + 1;
            return r % range + min;
        }
    } // namespace randy
} // namespace shard
