#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdint>
#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <map>

namespace shard{
    struct Timer{
        float waitTime = 0.0f;
        float elapsed  = 0.0f;
        bool oneShot   = false;
        std::function<void(Timer&)> callback={};
        bool finished = false;
    };
    struct Time{
        float dt=0.0f;
        float elapsed=0.0f;
        uint32_t ticks=0;
        float fps=0.0f;
        std::map<std::string, Timer> timers={};
    };
    struct TimeStamp{
        float taken = 0.0f;
        float ended = 0.0f;
        float time  = 0.0f;
        std::string name = "Time Stamp";
        TimeStamp(){}
        TimeStamp(const std::string& _name):
            name{_name}
        {}
        void take(){
            taken = static_cast<float>(glfwGetTime());
        }
        void end(){
            ended = static_cast<float>(glfwGetTime());
            time = ended - taken;
        }
        void endAndDisplay(){
            end();
            display();
        }
        void display(){
            std::cout << "Time Stamp -> " << name << ": Time elapsed " << time << "s\n";
        }
    };
    namespace time{
        void updateTime(Time& time);
        void addTimer(
            Time& time, const std::string& name, float waitTime, std::function<void(Timer&)> callback, bool oneShot=false
        );
        void resetTimer(Time& time, const std::string& name);
        Timer& getTimer(Time& time, const std::string& name);
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