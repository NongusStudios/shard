#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdint>
#include <cassert>
#include <vector>
#include <fstream>
#include <iostream>

#include "def.hpp"

#define shard_abort_ifnot(expr) \
    if(!static_cast<bool>(expr)){ \
        std::cerr << SHARD_FUNC << " Aborted -> (" << #expr << ") failed.\n"; \
        std::abort(); \
    }
#define shard_log_and_abort(msg) \
    std::cerr << SHARD_FUNC << " Aborted -> " << msg << ".\n"; \
    std::abort()

#define shard_delete_copy_constructors(className) \
    className(const className&) = delete; \
    className& operator = (const className&) = delete

namespace shard{
    inline uint32_t makeVersion(uint32_t major, uint32_t minor, uint32_t patch){
        return VK_MAKE_VERSION(major, minor, patch);
    }
    inline std::vector<char> readSPVfile(const char* filePath){
        std::ifstream fp(filePath, std::ios::ate | std::ios::binary);
        assert(fp.is_open() && "File does not exist!");
        size_t fileSize = static_cast<size_t>(fp.tellg());
        std::vector<char> buf(fileSize);
        fp.seekg(0);
        fp.read(buf.data(), fileSize);
        fp.close();
        return buf;
    }
    inline VkExtent2D getWindowExtent(GLFWwindow* window){
        assert(window != nullptr);
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        return {static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
    }
    inline VkExtent2D getFramebufferExtent(GLFWwindow* window){
        assert(window != nullptr);
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        return {static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
    }
    inline glm::vec2 getCursorPos(GLFWwindow* window){
        assert(window != nullptr);
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return glm::vec2(float(x), float(y));
    }
    inline uint32_t calculateMipmapLevels(uint32_t w, uint32_t h){
        return uint32_t(
                std::floor(std::log2(std::max(w, h)))
        ) + 1;
    }
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