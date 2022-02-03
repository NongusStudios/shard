#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdint>
#include <iostream>

namespace shard{
    struct Time{
        float dt=0.0f;
        float elapsed=0.0f;
        uint32_t ticks=0;
        float fps=0.0f;
    };
    struct TimeStamp{
        float taken;
        float ended;
        float time;
        std::string name = "Time Stamp";
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