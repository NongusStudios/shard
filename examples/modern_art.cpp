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
        const uint32_t triCount = 1000;
        std::vector<shard::gfx::Vertex2D> triVertices;
        glm::vec2 triPos = {0.0f, 0.0f};
        float triRot = 0.0f;

        int width = 800;
        int height = 600;

        GLFWwindow* window = nullptr;
        shard::gfx::Graphics gfx;
        shard::gfx::DescriptorPool descPool;
        shard::gfx::DescriptorSetLayout descLayout;
        std::vector<VkDescriptorSet> descSets{shard::gfx::Swapchain::MAX_FRAMES_IN_FLIGHT};
        VkPipelineLayout triPipelineLayout;
        shard::gfx::Pipeline triPipeline;
        shard::gfx::Buffer triStagingBuffer;
        shard::gfx::Buffer triVertexBuffer;
        std::vector<shard::gfx::Buffer> uniformBuffers;
        shard::gfx::Image skylineImage;
        shard::gfx::Image faceImage;
        shard::gfx::Sampler skylineSampler;
        shard::gfx::Sampler faceSampler;
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

            uint32_t left;
            uint32_t right;
            uint32_t up;
            uint32_t down;
            uint32_t rotate_left;
            uint32_t rotate_right;
        } actions;

        enum InputLayer{
            INPUT_LAYER_BASE,
            INPUT_LAYER_ONGROUND,
        };

        VkPipelineLayout createTriPipelineLayout(){
            VkPipelineLayout layout = VK_NULL_HANDLE;

            VkDescriptorSetLayout descLayouts[] = {descLayout.layout()};

            VkPipelineLayoutCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            createInfo.setLayoutCount = 1;
            createInfo.pSetLayouts = descLayouts;

            shard_abort_ifnot(
                vkCreatePipelineLayout(
                    gfx.device().device(), &createInfo, nullptr, &layout
                ) == VK_SUCCESS
            );

            return layout;
        }
    public:
        App():
            window{createWindow()},
            gfx{window, true},
            descPool{
                shard::gfx::DescriptorPool::Builder(gfx.device())
                    .setMaxSets(shard::gfx::Swapchain::MAX_FRAMES_IN_FLIGHT)
                    .addPoolSize(
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        shard::gfx::Swapchain::MAX_FRAMES_IN_FLIGHT
                    )
                    .addPoolSize(
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        2*shard::gfx::Swapchain::MAX_FRAMES_IN_FLIGHT
                    )
                    .build()
            },
            descLayout{
                shard::gfx::DescriptorSetLayout::Builder(gfx.device())
                    .addBinding(
                        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_VERTEX_BIT
                    )
                    .addBinding(
                        1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT
                    )
                    .addBinding(
                        2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT 
                    )
                    .build()
            },
            triPipelineLayout{
                createTriPipelineLayout()
            },
            triPipeline{
                gfx.createPipeline(
                    triPipelineLayout,
                    "examples/shaders/tri.vert.spv",
                    "examples/shaders/tri.frag.spv",
                    {shard::gfx::Vertex2D::bindingDesc(VK_VERTEX_INPUT_RATE_VERTEX)},
                    shard::gfx::Vertex2D::attributeDescs(),
                    gfx.deafultPipelineConfig()
                )
            },
            triStagingBuffer{
                gfx.device(),
                sizeof(shard::gfx::Vertex2D)*triCount*3,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VMA_MEMORY_USAGE_CPU_ONLY,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            },
            triVertexBuffer{
                gfx.device(),
                sizeof(shard::gfx::Vertex2D)*triCount*3,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VMA_MEMORY_USAGE_GPU_ONLY,
                0
            },
            skylineImage{
                gfx.device(),
                "res/skyline.jpg"
            },
            faceImage{
                gfx.device(),
                "res/face.jpg"
            },
            skylineSampler{
                gfx.device(),
                VK_FILTER_NEAREST,
                VK_FILTER_NEAREST,
                VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VK_TRUE,
                VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                VK_SAMPLER_MIPMAP_MODE_LINEAR,
                skylineImage.mipMapLevels()
            },
            faceSampler{
                gfx.device(),
                VK_FILTER_NEAREST,
                VK_FILTER_NEAREST,
                VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VK_TRUE,
                VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                VK_SAMPLER_MIPMAP_MODE_LINEAR,
                faceImage.mipMapLevels()
            },
            input{window}
        {
            triVertices.resize(triCount*3);
            actions.exit = input.addAction(shard::Key::ESCAPE, INPUT_LAYER_BASE);
            actions.regenTriangles = input.addAction(shard::Key::R, INPUT_LAYER_BASE);
            actions.left = input.addAction(shard::Key::A , INPUT_LAYER_BASE);
            actions.right = input.addAction(shard::Key::D , INPUT_LAYER_BASE);
            actions.up = input.addAction(shard::Key::W , INPUT_LAYER_BASE);
            actions.down = input.addAction(shard::Key::S , INPUT_LAYER_BASE);
            actions.rotate_left = input.addAction(shard::Key::Q , INPUT_LAYER_BASE);
            actions.rotate_right = input.addAction(shard::Key::E , INPUT_LAYER_BASE);

            for(size_t i = 0; i < shard::gfx::Swapchain::MAX_FRAMES_IN_FLIGHT; i++){
                uniformBuffers.push_back(
                    gfx.createUniformBuffer(
                        sizeof(AppUbo),
                        nullptr
                    )
                );
                uniformBuffers[i].map();
            }

            for(size_t i = 0; i < descSets.size(); i++){
                auto bufferInfo = uniformBuffers[i].descriptorInfo();
                auto faceImageInfo = faceImage.descriptorInfo(faceSampler);
                auto skylineImageInfo = skylineImage.descriptorInfo(skylineSampler);
                
                shard::gfx::DescriptorWriter(descLayout, descPool)
                    .writeBuffer(0, &bufferInfo)
                    .writeImage(1, &faceImageInfo)
                    .writeImage(2, &skylineImageInfo)
                    .build(descSets[i]);
            }

            triStagingBuffer.map();
            genTriangles();
        }
        ~App(){
            glfwDestroyWindow(window);
        }
        
        void genTriangles(){
            std::srand((uint32_t)std::time(NULL));
            for(size_t i = 0; i < triCount; i+=3){
                //float randy_depth = std::clamp((float(std::rand())/float((RAND_MAX)) * 1.0f), 0.1f, 0.99999f);
                for(size_t j = 0; j < 3; j++){
                    float randy_color[3] = {
                        std::clamp((float(std::rand())/float((RAND_MAX)) * 1.0f), 0.2f, 1.0f),
                        std::clamp((float(std::rand())/float((RAND_MAX)) * 1.0f), 0.2f, 1.0f),
                        std::clamp((float(std::rand())/float((RAND_MAX)) * 1.0f), 0.2f, 1.0f)
                    };
                    float randy_pos[2] = {
                        std::clamp((float(std::rand())/float((RAND_MAX)) * 2.0f) - 1.0f, -1.0f, 1.0f),
                        std::clamp((float(std::rand())/float((RAND_MAX)) * 2.0f) - 1.0f, -1.0f, 1.0f)
                    };
                    glm::vec2 uvs[3] = {
                        {0.5f, 0.0f},
                        {0.0f, 1.0f},
                        {1.0f, 1.0f}
                    };
                    shard::gfx::Vertex2D v;
                    v.pos = { randy_pos[0], randy_pos[1]};
                    v.uv = uvs[j];
                    v.color = { randy_color[0], randy_color[1], randy_color[2], 1.0f};
                    triVertices[i+j] = v;
                }
            }
            gfx.device().waitIdle();
            void* data = triStagingBuffer.mappedMemory();
            memcpy(data, triVertices.data(), triStagingBuffer.size());
            triStagingBuffer.flush();

            gfx.device().copyBuffer(
                triStagingBuffer.buffer(),
                triVertexBuffer.buffer(),
                triVertexBuffer.size()
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

            const float vel = 0.5f;
            const float rotVel = 75.0f;

            switch(input.actionPressed(actions.left)){
                case shard::input::Result::TRUE:
                    triPos.x -= vel * time.dt;
                    //std::cout << "Left\n";
                    break;
                case shard::input::Result::FALSE:
                    break;
                case shard::input::Result::DISABLED:
                    break;
            }
            switch(input.actionPressed(actions.right)){
                case shard::input::Result::TRUE:
                    triPos.x += vel * time.dt;
                    //std::cout << "Right\n";
                    break;
                case shard::input::Result::FALSE:
                    break;
                case shard::input::Result::DISABLED:
                    break;
            }
            switch(input.actionPressed(actions.down)){
                case shard::input::Result::TRUE:
                    triPos.y += vel * time.dt;
                    //std::cout << "Down\n";
                    break;
                case shard::input::Result::FALSE:
                    break;
                case shard::input::Result::DISABLED:
                    break;
            }
            switch(input.actionPressed(actions.up)){
                case shard::input::Result::TRUE:
                    triPos.y -= vel * time.dt;
                    //std::cout << "Up\n";
                    break;
                case shard::input::Result::FALSE:
                    break;
                case shard::input::Result::DISABLED:
                    break;
            }
            switch(input.actionPressed(actions.rotate_left)){
                case shard::input::Result::TRUE:
                    triRot -= rotVel * time.dt;
                    //std::cout << "rLeft\n";
                    break;
                case shard::input::Result::FALSE:
                    break;
                case shard::input::Result::DISABLED:
                    break;
            }
            switch(input.actionPressed(actions.rotate_right)){
                case shard::input::Result::TRUE:
                    triRot += rotVel * time.dt;
                    //std::cout << "rRight\n";
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
            if(VkCommandBuffer commandBuffer = gfx.beginRenderPass(
                shard::gfx::Color(44.0f, 44.0f, 44.0f)
            )){
                AppUbo ubo = {};
                ubo.proj = glm::ortho(
                    -1.0f, 1.0f, -1.0f, 1.0f,
                    -255.0f, 255.0f
                );
                ubo.view = glm::mat4(1.0f);
                ubo.model = glm::mat4(1.0f);

                ubo.model = glm::translate(ubo.model, {triPos, 0.0f});
                ubo.model = glm::rotate(
                    ubo.model, glm::radians(triRot), {0.0f, 0.0f, 1.0f}
                );
                ubo.model = glm::scale(ubo.model, {1.5f, 1.5f, 1.5f});

                memcpy(
                    uniformBuffers[gfx.frameIndex()].mappedMemory(),
                    &ubo,
                    sizeof(AppUbo)
                );

                vkCmdBindDescriptorSets(
                    commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    triPipelineLayout,
                    0, 1,
                    &descSets[gfx.frameIndex()],
                    0, nullptr
                );

                triPipeline.bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
                triVertexBuffer.bindVertex(commandBuffer);
                vkCmdDraw(commandBuffer, static_cast<uint32_t>(triVertices.size()), 1, 0, 0);
                gfx.endRenderPass();
            }
        }
        void cleanup(){
            vkDestroyPipelineLayout(
                gfx.device().device(), triPipelineLayout, nullptr
            );
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
            cleanup();
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