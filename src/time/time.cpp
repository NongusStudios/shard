#include <shard/time/time.hpp>

namespace shard{
    namespace time{
        void updateTime(Time& time){
            Time newT = {};
            newT.elapsed = glfwGetTime();
            newT.ticks = static_cast<uint32_t>(newT.elapsed*1000.0f);
            newT.dt = newT.elapsed - time.elapsed;
            newT.fps = 1.0f / newT.dt;
            time = newT;
        }
    } // namespace time
    
} // namespace shard
