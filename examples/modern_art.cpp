// Modules
#include <shard/input/input.hpp>
#include <shard/time/time.hpp>
#include <shard/gfx/gfx.hpp>

#include <iostream>
#include <memory>
#include <ctime>
#include <memory.h>

struct AppUbo{
    glm::mat4 proj  = glm::mat4(1.0f);
    glm::mat4 view  = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
};

class App{
    private:
        const uint32_t triCount = 200;
        std::vector<shard::gfx::Vertex> triVertices;
        AppUbo uboData;

        int width = 800;
        int height = 600;

        GLFWwindow* window = nullptr;
        shard::gfx::Graphics gfx;
        shard::gfx::Pipeline triPipeline;
        shard::gfx::Buffer triVertexBuffer;
        shard::gfx::Buffer triStagingBuffer;
        shard::gfx::Buffer uniformBuffer;
        shard::Input input;
        shard::Time time;

        GLFWwindow* createWindow(){
            shard_abort_ifnot(glfwInit());
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            GLFWwindow* win = glfwCreateWindow(
                width, height,
                "App",
                nullptr, nullptr
            );
            return win;
        }

        struct {
            uint32_t exit;
            uint32_t regenTriangles;
        } actions;

        enum InputLayer{
            INPUT_LAYER_BASE,
            INPUT_LAYER_ONGROUND,
        };
    public:
        App():
            window{createWindow()},
            gfx{window},
            triPipeline{
                gfx.createPipeline(
                    gfx.emptyPipelineLayout(),
                    "shaders/tri.vert.spv",
                    "shaders/tri.frag.spv",
                    {shard::gfx::Vertex::bindingDesc()},
                    shard::gfx::Vertex::attributeDescs(),
                    gfx.deafultPipelineConfig()
                )
            },
            triStagingBuffer{
                gfx.device(),
                sizeof(shard::gfx::Vertex)*triCount*3,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VMA_MEMORY_USAGE_CPU_ONLY,
                0
            },
            triVertexBuffer{
                gfx.device(),
                sizeof(shard::gfx::Vertex)*triCount*3,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VMA_MEMORY_USAGE_GPU_ONLY,
                0
            },
            uniformBuffer{
                gfx.createUniformBuffer(
                    sizeof(AppUbo),
                    &uboData
                )
            },
            input{window}
        {
            triVertices.resize(triCount*3);
            actions.exit = input.addAction(shard::Key::ESCAPE, INPUT_LAYER_BASE);
            actions.regenTriangles = input.addAction(shard::Key::R, INPUT_LAYER_BASE);

            triStagingBuffer.map();
            uniformBuffer.map();
            genTriangles();
        }
        ~App(){}
        
        void genTriangles(){
            std::srand((uint32_t)std::time(NULL));
            for(size_t i = 0; i < triCount; i+=3){
                float randy_depth = (float(std::rand())/float((RAND_MAX)) * 1.0f);
                for(size_t j = 0; j < 3; j++){
                    float randy_color[3] = {
                        std::clamp((float(std::rand())/float((RAND_MAX)) * 1.0f), 0.2f, 1.0f),
                        std::clamp((float(std::rand())/float((RAND_MAX)) * 1.0f), 0.2f, 1.0f),
                        std::clamp((float(std::rand())/float((RAND_MAX)) * 1.0f), 0.1f, 1.0f)
                    };
                    float randy_pos[2] = {
                        (float(std::rand())/float((RAND_MAX)) * 3.0f) - 1.5f,
                        (float(std::rand())/float((RAND_MAX)) * 3.0f) - 1.5f
                    };
                    shard::gfx::Vertex v;
                    v.pos = { randy_pos[0], randy_pos[1], randy_depth };
                    v.color = { randy_color[0], randy_color[1], randy_color[2]  };
                    triVertices[i+j] = v;
                }
            }
            gfx.device().waitIdle();
            void* data = triStagingBuffer.mappedMemory();
            memcpy(data, triVertices.data(), triVertices.size()*sizeof(shard::gfx::Vertex));
            triStagingBuffer.flush();

            gfx.device().copyBuffer(
                triStagingBuffer.buffer(),
                triVertexBuffer.buffer(),
                triVertices.size()*sizeof(shard::gfx::Vertex)
            );
        }
        
        void processInput(){
            switch(input.actionPressed(actions.exit)){
                case shard::input::Result::TRUE:
                    glfwSetWindowShouldClose(window, true);
                    break;
                case shard::input::Result::FALSE:
                    break;
                case shard::input::Result::DISABLED:
                    //std::cout << "Action(\"Exit\") is currently disabled!\n";
                    break;
            }
            switch(input.actionJustPressed(actions.regenTriangles)){
                case shard::input::Result::TRUE:
                    genTriangles();
                    break;
                case shard::input::Result::FALSE:
                    break;
                case shard::input::Result::DISABLED:
                    break;
            }
        }
        void update(){
        }
        void render(){
            if(VkCommandBuffer commandBuffer = gfx.beginRenderPass()){
                triPipeline.bind(commandBuffer);
                triVertexBuffer.bindVertex(commandBuffer);
                vkCmdDraw(commandBuffer, static_cast<uint32_t>(triVertices.size()), 1, 0, 0);
                gfx.endRenderPass();
            }
        }

        void run(){
            while(!glfwWindowShouldClose(window)){
                glfwPollEvents();
                shard::time::updateTime(time);
                
                processInput();
                update();
                render();
            }
            gfx.device().waitIdle();
        }
};

int main(){
    shard::gfx::PipelineConfigInfo pipelineConfig;
    pipelineConfig.makeDefault();
    std::unique_ptr<App> app = std::make_unique<App>();
    app->run();
    return 0;
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