#include <shard/renderer/renderer2d.hpp>
#include <shard/time/time.hpp>

class BasicRendering {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        BasicRendering():
            window{createWindow(WIDTH, HEIGHT)},
            _renderer{
                shard::r2d::Renderer::Builder(window)
                    .setExtent({uint32_t(WIDTH), uint32_t(HEIGHT)})
                    .setTexturePoolSize(50)
                    .setVsync(true)
                    .build()
            }
        {}
        ~BasicRendering(){}

        void render(){
            if(renderer().beginRenderPass(shard::gfx::Color(44.0f))){
                // TODO
                /*renderer()
                    .drawRect(shard::r2d::Rect(
                        {200.0f, -60.0f}, 0.0f, {32.0f, 32.0f},
                        shard::gfx::Color(252.0f, 8.0f, 104.0f)
                    ))
                    .drawRectWithBorder(shard::r2d::Rect(
                        {-200.0f, 60.0f}, 45.0f, {48.0f, 48.0f},
                        shard::gfx::Color(8.0f, 252.0f, 104.0f)
                    ), shard::gfx::Color(0.0f), 1.0f)
                    .drawCircle(shard::r2d::Circle(
                        {0.0f, 0.0f}, 16.0f, shard::gfx::Color(255.0f)
                    ));
                */
                renderer().endRenderPass();
            }
        }
        void run(){
            while(!glfwWindowShouldClose(window)){
                glfwPollEvents();
                shard::time::updateTime(time);
                render();
            }
            renderer().waitIdle();
        }
        
        shard::r2d::Renderer& renderer(){ return *_renderer; }
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
        std::unique_ptr<shard::r2d::Renderer> _renderer;
        shard::Time time;
};

int main(){
    BasicRendering br;
    br.run();
    return 0;
}