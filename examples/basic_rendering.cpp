#include <shard/renderer/renderer2d.hpp>
#include <shard/random/random.hpp>
#include <shard/time/time.hpp>

class BasicRendering{
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;
        static constexpr uint32_t RECT_COUNT = 100;

        BasicRendering():
            window{createWindow(WIDTH, HEIGHT)},
            r2d{window, {WIDTH, HEIGHT}, 50, true}
        {
            shard::time::updateTime(time);
            shard::randy::Random rand(time.ticks);

            middleRect = r2d.addRect(
                {-300.0f, 200.0f},
                0.0f,
                {32.0f, 32.0f},
                {255.0f},
                100.0f,
                0.04f,
                {0.0f}
            );

            rects.resize(RECT_COUNT);
            for(auto& rect : rects){
                rect = r2d.addRect(
                    {
                        rand.randRangef(-float(WIDTH/2 ), float(WIDTH/2)),
                        rand.randRangef(-float(HEIGHT/2), float(HEIGHT/2))
                    },
                    glm::degrees(rand.randRangef(0.0f, 360.0f)),
                    {
                        rand.randRangef(1.0f, 100.0f),
                        rand.randRangef(1.0f, 100.0f)
                    },
                    {
                        rand.randRangef(0.0f, 255.0f),
                        rand.randRangef(0.0f, 255.0f),
                        rand.randRangef(0.0f, 255.0f)
                    },
                    rand.randRangef(-99.0f, 99.0f)
                );
            }
        }
        ~BasicRendering(){}

        void render(){
            shard::r2d::Rect& mRect = r2d.getRect(middleRect);
            mRect.rotation += (glm::pi<float>()/2.0f)*time.dt;

            if(r2d.startFrame({44.0f})){
                for(auto& rect : rects){
                    r2d.drawRect(rect);
                }
                r2d.drawRect(middleRect);
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

        uint32_t middleRect;
        std::vector<uint32_t> rects;
};

int main(){
    BasicRendering br;
    br.run();
    return 0;
}