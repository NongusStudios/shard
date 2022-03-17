#include <shard/time/time.hpp>

#include <cassert>

namespace shard{
    namespace time{
        void updateTime(Time& time){
            Time newT = {};
            newT.elapsed = static_cast<float>(glfwGetTime());
            newT.ticks = static_cast<uint32_t>(newT.elapsed*1000.0f);
            newT.dt = newT.elapsed - time.elapsed;
            newT.fps = 1.0f / newT.dt;
            newT.timers = std::move(time.timers);
            time = std::move(newT);

            for(auto& timerPair : time.timers){
                auto& timer = timerPair.second;
                timer.elapsed += time.dt;
                if(timer.elapsed >= timer.waitTime && !timer.finished){
                    timer.callback(timer);
                    timer.elapsed = 0.0f;
                    if(timer.oneShot) timer.finished = true;
                }
            }
        }
        void addTimer(Time& time, const std::string& name, float waitTime, std::function<void(Timer&)> callback, bool oneShot){
            assert(!time.timers.contains(name));
            time.timers[name] = Timer(
                waitTime,
                0.0f,
                oneShot,
                callback
            );
        }
        void resetTimer(Time& time, const std::string& name){
            assert(time.timers.contains(name));
            auto& timer = time.timers[name];
            timer.elapsed = 0.0f;
            timer.finished = false;
        }
        Timer& getTimer(Time& time, const std::string& name){
            assert(time.timers.contains(name));
            return time.timers[name];
        }
    } // namespace time
    
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
