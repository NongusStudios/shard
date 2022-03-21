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
void rectSystem(shard::ECS& ecs, const shard::Entity& entity, shard::Component* component){
    Rect& rect = *reinterpret_cast<Rect*>(component);
    Transform& transform = ecs.getComponentArray<Transform>("transforms").get(entity);
    shard::r2d::Renderer& r2d = ecs.getResource<shard::r2d::Renderer>("r2d");

    shard::r2d::Rect& r2dRect = r2d.getRect(rect.rect);
    r2dRect.position = transform.position;
    r2dRect.rotation = transform.rotation;
    r2dRect.scale = transform.scale;

    r2d.drawRect(rect.rect);
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
        shard::ComponentArray<Rect> rects;
            // Resources
        ecs .insertResource<shard::Time>("time", time)
            .insertResource<GLFWwindow*>("window", window)
            .insertConstantResource<int>("WIDTH", WIDTH)
            .insertConstantResource<int>("HEIGHT", HEIGHT)
            .insertResource<shard::r2d::Renderer>("r2d", r2d)
            .insertResource<shard::Sound>("sound", sound)
            .insertResource<shard::Input>("input", input)
            // Component Arrays
            .insertComponentArray<Transform>("transforms", transforms)
            .insertComponentArray<Rect>("rects", rects)
            // Systems
            .insertSystem("transform", "transforms", transformSystem)
            .insertSystem("rect", "rects", rectSystem);
        
        std::vector<shard::Entity> entities(10);
        float offset = 10.0f;
        for(auto& entity : entities){
            entity = ecs.addEntity();
            ecs.insertComponent<Transform>(entity, "transforms")
               .insertComponent<Rect>     (entity, "rects");
            auto& component = ecs.getComponentArray<Transform>("transforms").get(entity);
            component.position = {0.0f, 0.0f};
            component.scale = {32.0f, 32.0f};
            component.vel = {100.0f-offset, 0.0f};
            offset += 20.0f;
        }

        while(!glfwWindowShouldClose(window)){
            glfwPollEvents();
            shard::time::updateTime(time);
            if(r2d.startFrame({44.0f})){
                ecs.update<Transform>("transform")
                   .update<Rect>     ("rect");
                r2d.endFrame();
            }
        }
        
        r2d.cleanup();
    }
    glfwDestroyWindow(window);
}