#include "components.hpp"

GLFWwindow* createWindow(int w, int h){
    shard_abort_ifnot(glfwInit());
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* win = glfwCreateWindow(
        w, h,
        "Breakout",
        nullptr, nullptr
    );
    return win;
}

void transformSystem(shard::ECS& ecs, const shard::Entity& entity, shard::Component* component){
    Transform& transform = *reinterpret_cast<Transform*>(component);
    shard::Time& time = ecs.getResource<shard::Time>("time");
    transform.vel      += transform.accel * time.dt;
    transform.position += transform.vel   * time.dt;
    transform.rotation += transform.angularVel * time.dt;
}

int main(){
    const int WIDTH=800, HEIGHT=600;
    GLFWwindow* window = createWindow(WIDTH, HEIGHT);
    {
        shard::ECS ecs;
        shard::Time time = {};
        shard::r2d::Renderer r2d(window, {WIDTH, HEIGHT}, true);
        shard::Sound sound;
        shard::Input input(window);

        shard::ComponentArray<Transform> transforms;

        ecs.insertResource<shard::Time>("time", time)
            .insertResource<GLFWwindow*>("window", window)
            .insertConstantResource<int>("WIDTH", WIDTH)
            .insertConstantResource<int>("HEIGHT", HEIGHT)
            .insertResource<shard::r2d::Renderer>("r2d", r2d)
            .insertResource<shard::Sound>("sound", sound)
            .insertResource<shard::Input>("input", input)
            .insertComponentArray<Transform>("transforms", transforms)
            .insertSystem("transform", "transforms", transformSystem);
        
        while(!glfwWindowShouldClose(window)){
            glfwPollEvents();
            shard::time::updateTime(time);
            if(r2d.startFrame({44.0f})){
                ecs.update();
                r2d.endFrame();
            }
        }
        
        r2d.cleanup();
    }
    glfwDestroyWindow(window);
}