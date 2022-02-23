#include <shard/renderer/renderer2d.hpp>

class BasicRendering {
    public:
        BasicRendering():
            window{createWindow(800, 600)}
        {}
        ~BasicRendering(){}
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
};

int main(){

    return 0;
}