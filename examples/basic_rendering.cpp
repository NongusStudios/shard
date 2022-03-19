#include <shard/renderer/renderer2d.hpp>
#include <shard/time/time.hpp>

class BasicRendering{
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;
        static constexpr uint32_t RECT_COUNT = 100;

        BasicRendering():
            window{createWindow(WIDTH, HEIGHT)},
            r2d{window, {WIDTH, HEIGHT}, true}
        {
            shard::time::updateTime(time);

            face_jpg = r2d.addTexture("res/face.jpg", VK_FILTER_LINEAR);
            faceSprite = r2d.addSprite(
                face_jpg,
                {0.0f, 0.0f},
                0.0f,
                {float(WIDTH), float(HEIGHT)},
                shard::gfx::Color(255.0f),
                -99.9f
            );

            rect = r2d.addRect(
                {0.0f, 0.0f},
                0.0f,
                {32.0f, 32.0f},
                {255.0f},
                100.0f,
                0.04f,
                {0.0f}
            );
        }
        ~BasicRendering(){}

        void render(){
            shard::r2d::Rect& mRect = r2d.getRect(rect);
            mRect.rotation += (glm::pi<float>()/2.0f)*time.dt;

            if(r2d.startFrame({44.0f})){
                r2d.drawRect(rect);
                r2d.drawSprite(faceSprite);
                r2d.endFrame();
            }
        }
        void run(){
            while(!glfwWindowShouldClose(window)){
                glfwPollEvents();
                shard::time::updateTime(time);
                render();
            }
            r2d.cleanup();
        }
    private:
        GLFWwindow* createWindow(int width, int height){
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
        GLFWwindow* window;
        shard::r2d::Renderer r2d;
        shard::Time time;

        uint32_t rect;
        uint32_t face_jpg;
        uint32_t faceSprite;
        std::vector<uint32_t> rects;
};

int main(){
    BasicRendering br;
    br.run();
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