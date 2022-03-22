#include <shard/renderer/renderer2d.hpp>
#include <shard/time/time.hpp>
#include <shard/random/random.hpp>
#include <shard/input/input.hpp>
#include <shard/sound/sound.hpp>
#include <shard/ecs/ecs.hpp>

using namespace shard;

namespace component{
    class Transform : public Component {
        public:
            Transform(){}
            Transform(
                const glm::vec2& _pos,
                float rot,
                const glm::vec2& _scale,
                const glm::vec2& _vel
            ):
                pos{_pos},
                rotation{rot},
                scale{_scale},
                vel{_vel}
            {}

            void create(ECS& ecs, const Entity& entity, const Component* init) override {
                const Transform& initData = *reinterpret_cast<const Transform*>(init);
                pos      = initData.pos;
                rotation = initData.rotation;
                scale    = initData.scale;
                vel      = initData.vel;
            }
            
            glm::vec2 pos;
            float rotation;
            glm::vec2 scale;
            glm::vec2 vel;
    };
    class Rect : public Component {
        public:
            Rect(){}
            Rect(
                const gfx::Color& _color,
                float _zindex
            ):
                color{_color},
                zindex{_zindex}
            {}

            void create(ECS& ecs, const Entity& entity, const Component* init) override {
                const Rect& initData = *reinterpret_cast<const Rect*>(init);
                r2d::Renderer& r2d = ecs.getResource<r2d::Renderer>("r2d");
                Transform& transform = ecs.getComponentArray<Transform>("transforms").get(entity);
                color = initData.color;
                zindex = initData.zindex;
                rect = r2d.addRect(
                    transform.pos, transform.rotation,
                    transform.scale,
                    color, zindex
                );
            }
            void destroy(ECS& ecs, const Entity& entity) override {
                r2d::Renderer& r2d = ecs.getResource<r2d::Renderer>("r2d");
                r2d.removeRect(rect);
            }

            gfx::Color color;
            float      zindex;
            bool       visible = true;
            uint32_t   rect;
    };
    class Sprite : public Component {
        public:
            Sprite(){}
            Sprite(
                const std::string& _texture,
                const gfx::Color& _color,
                float _zindex
            ):
                texture{_texture},
                color{_color},
                zindex{_zindex}
            {}

            void create(ECS& ecs, const Entity& entity, const Component* init) override {
                const Sprite& initData = *reinterpret_cast<const Sprite*>(init);
                r2d::Renderer& r2d = ecs.getResource<r2d::Renderer>("r2d");
                Transform& transform = ecs.getComponentArray<Transform>("transforms").get(entity);
                texture = initData.texture;
                zindex  = initData.zindex;
                uint32_t t = ecs.getResource<uint32_t>(texture);
                color   = initData.color;
                sprite  = r2d.addSprite(
                    t, transform.pos,
                    transform.rotation,
                    transform.scale,
                    color, zindex
                );
            }
            void destroy(ECS& ecs, const Entity& entity) override {
                r2d::Renderer& r2d = ecs.getResource<r2d::Renderer>("r2d");
                r2d.removeSprite(sprite);
            }
        
            std::string texture;
            gfx::Color  color;
            float       zindex  = 0.0f;
            glm::vec2   srcPos  =  {0.5f, 0.5f};
            glm::vec2   srcSize = {1.0f, 1.0f};
            bool        visible = true;
            uint32_t    sprite;
    };
    class Controller : public Component {
        public:
            Controller(){}
            Controller(Key _left, Key _right):
                left{_left},
                right{_right}
            {}

            void create(ECS& ecs, const Entity& entity, const Component* init) override {
                const Controller& initData = *reinterpret_cast<const Controller*>(init);
                left  = initData.left;
                right = initData.right;
            }

            Key left;
            Key right;
    };
    class Paddle : public Component {
        public:
            void create(ECS& ecs, const Entity& entity, const Component* init) override {
                Controller& controller = ecs.getComponentArray<Controller>("controllers").get(entity);
                Input& input = ecs.getResource<Input>("input");
                left  = input.addAction(controller.left, 0);
                right = input.addAction(controller.right, 0);
                reset = input.addAction(Key::R, 0);
            }

            uint32_t left;
            uint32_t right;
            uint32_t reset;
    };

    class Ball : public Component {};

    enum class ColliderID{
        PLAYER,
        BLOCK
    };
    class Collider : public Component {
        public:
            Collider(){}
            Collider(ColliderID _id):
                id{_id}
            {}
            void create(ECS& ecs, const Entity& entity, const Component* init) override {
                const Collider& initData = *reinterpret_cast<const Collider*>(init);
                id = initData.id;
            }
            ColliderID id = ColliderID::BLOCK;
            bool active = true;
    };

    class Score : public Component {
        public:
            void create(ECS& ecs, const Entity& entity, const Component* init) override {
                r2d::Renderer& r2d = ecs.getResource<r2d::Renderer>("r2d");
                youWonSprite = r2d.addSprite(
                    ecs.getResource<uint32_t>("youwinTexture"),
                    {0.0f, 0.0f},
                    0.0f,
                    {128.0f, 32.0f},
                    {255.0f}, 99.0f
                );
            }
            void destroy(ECS& ecs, const Entity& entity) override {
                r2d::Renderer& r2d = ecs.getResource<r2d::Renderer>("r2d");
                r2d.removeSprite(youWonSprite);
            }
            uint32_t youWonSprite;
    };
}

struct GameState{
    bool hasWon    = false;
    bool hasLost   = false;
};

void reset(ECS& ecs){
    GameState& gamestate  = ecs.getResource<GameState>("gamestate");
    const float& BALL_VEL = ecs.getConstantResource<float>("BALL_VEL");
    gamestate.hasLost = false;
    gamestate.hasWon  = false;

    auto colliders = ecs.getComponentArray<component::Collider>("colliders").getComponents();
    for(auto& [cEntity, collider] : colliders){
        collider.active = true;
        component::Rect& cRect = ecs.getComponentArray<component::Rect>("rects").get(cEntity);
        cRect.visible = true;
        Entity ball  = ecs.getComponentArray<component::Ball>("balls").getEntityFromIndex(0);
        component::Transform& ballTransform = ecs.getComponentArray<component::Transform>("transforms")
                                                    .get(ball);
        component::Rect& ballRect = ecs.getComponentArray<component::Rect>("rects")
                                                    .get(ball);
        ballTransform.pos = {0.0f, 0.0f};
        ballTransform.vel = {0.0f, -BALL_VEL};
        ballRect.visible = true;

        Entity player = ecs.getComponentArray<component::Transform>("paddles").getEntityFromIndex(0);
        ecs.getComponentArray<component::Transform>("transforms").get(player).pos.x = 0.0f;

        ecs.getResource<uint32_t>("destroyed") = 0;
    }
}

void transformSystem(ECS& ecs, const Entity& entity, Component* component){
    GameState& gamestate = ecs.getResource<GameState>("gamestate");
    if(gamestate.hasWon) return;

    component::Transform& transform = *reinterpret_cast<component::Transform*>(component);
    Time& time = ecs.getResource<Time>("time");
    transform.pos += transform.vel * time.dt;
}
void rectSystem(ECS& ecs, const Entity& entity, Component* component){
    component::Rect& rect = *reinterpret_cast<component::Rect*>(component);
    component::Transform& transform = ecs.getComponentArray<component::Transform>("transforms").get(entity);
    r2d::Renderer& r2d = ecs.getResource<r2d::Renderer>("r2d");

    r2d::Rect& rect2d = r2d.getRect(rect.rect);
    rect2d.position = transform.pos;
    rect2d.rotation = transform.rotation;
    rect2d.scale = transform.scale;
    rect2d.color = rect.color;
    rect2d.zindex = rect.zindex;

    if(rect.visible)
        r2d.drawRect(rect.rect);
}
void spriteSystem(ECS& ecs, const Entity& entity, Component* component){
    component::Sprite& sprite = *reinterpret_cast<component::Sprite*>(component);
    component::Transform& transform = ecs.getComponentArray<component::Transform>("transforms").get(entity);
    r2d::Renderer& r2d = ecs.getResource<r2d::Renderer>("r2d");

    r2d::Sprite& sprite2d = r2d.getSprite(sprite.sprite);
    sprite2d.color    = sprite.color;
    sprite2d.zindex   = sprite.zindex;
    sprite2d.position = transform.pos;
    sprite2d.rotation = transform.rotation;
    sprite2d.scale    = transform.scale;

    if(sprite.visible)
        r2d.drawSprite(sprite.sprite);
}
void paddleSystem(ECS& ecs, const Entity& entity, Component* component){
    component::Paddle& paddle = *reinterpret_cast<component::Paddle*>(component);
    Input& input = ecs.getResource<Input>("input");
    component::Transform& transform = ecs.getComponentArray<component::Transform>("transforms").get(entity);

    const float PADDLE_VEL = 300.0f;

    bool l=false,r=false;
    switch(input.actionPressed(paddle.left)){
        case input::Result::TRUE:
            if(input.actionPressed(paddle.right) != input::Result::TRUE){
                transform.vel.x = -PADDLE_VEL;
                l = true;
            }
        default:
            break;
    }
    switch(input.actionPressed(paddle.right)){
        case input::Result::TRUE:
            if(input.actionPressed(paddle.left) != input::Result::TRUE){
                transform.vel.x = PADDLE_VEL;
                r = true;
            }
        default:
            break;
    }
    switch(input.actionJustPressed(paddle.reset)){
        case input::Result::TRUE:
            reset(ecs);
            break;
        default:
            break;
    }
    if(!r && !l) transform.vel.x = 0.0f;
    
    const int& WIDTH = ecs.getConstantResource<int>("WIDTH");

    transform.pos.x = std::clamp(transform.pos.x, 
        -float(WIDTH/2)+transform.scale.x/2.0f,
         float(WIDTH/2)-transform.scale.x/2.0f
    );
}
bool checkCollision(glm::vec2 pos1, glm::vec2 size1, glm::vec2 pos2, glm::vec2 size2){
    pos1 = {pos1.x-size1.x/2.0f, pos1.y-size1.y/2.0f};
    pos2 = {pos2.x-size2.x/2.0f, pos2.y-size2.y/2.0f};
    return  pos1.x  < pos2.x  + size2.x &&
            pos1.x  + size1.x > pos2.x  &&
            pos1.y  < pos2.y  + size2.y &&
            size2.y + pos1.y  > pos2.y;
}
void ballSystem(ECS& ecs, const Entity& entity, Component* component){
    randy::Random& rng = ecs.getResource<randy::Random>("rng");
    component::Transform& transform = ecs.getComponentArray<component::Transform>("transforms").get(entity);
    component::Rect& rect = ecs.getComponentArray<component::Rect>("rects").get(entity);
    Sound& sound = ecs.getResource<Sound>("sound");

    const int& WIDTH = ecs.getConstantResource<int>("WIDTH");
    const int& HEIGHT = ecs.getConstantResource<int>("HEIGHT");
    const uint32_t& BLOCK_COUNT = ecs.getConstantResource<uint32_t>("BLOCK_COUNT");
    uint32_t& destroyed = ecs.getResource<uint32_t>("destroyed");

    if(
        transform.pos.x <= -float(WIDTH/2)+transform.scale.x/2.0f ||
        transform.pos.x >=  float(WIDTH/2)-transform.scale.x/2.0f
    ){
        transform.vel.x *= -1.0f;
        sound.play("res/hit.wav");
        //rect.color = {rng.randRangef(0.0f, 255.0f), rng.randRangef(0.0f, 255.0f), rng.randRangef(0.0f, 255.0f)};
    } if(
        transform.pos.y >=  float(HEIGHT/2)-transform.scale.y/2.0f
    ){
        transform.vel.y *= -1.0f;
        sound.play("res/hit.wav");
        //rect.color = {rng.randRangef(0.0f, 255.0f), rng.randRangef(0.0f, 255.0f), rng.randRangef(0.0f, 255.0f)};
    }

    GameState& gamestate = ecs.getResource<GameState>("gamestate");
    if(
        transform.pos.y <=  -float(HEIGHT/2)+transform.scale.y/2.0f
    ){
        if(!gamestate.hasWon)
            reset(ecs);
    }

    auto colliders = ecs.getComponentArray<component::Collider>("colliders").getComponents();
    for(auto& [cEntity, collider] : colliders){
        if(!collider.active) continue;
        component::Transform& cTransform = ecs.getComponentArray<component::Transform>("transforms").get(cEntity);
        if(checkCollision(cTransform.pos, cTransform.scale, transform.pos, transform.scale)){
            if(transform.vel.x == 0.0f){
                const float& BALL_VEL = ecs.getConstantResource<float>("BALL_VEL");
                float v = rng.randRangef(BALL_VEL*0.5f, BALL_VEL*1.5f);
                int n = rng.randRange(0, 1);
                transform.vel.x = n ? v : -v;
            }
            transform.vel.y *= -1.0f;
            component::Rect& cRect = ecs.getComponentArray<component::Rect>("rects").get(cEntity);
            rect.color = cRect.color;

            if(collider.id == component::ColliderID::BLOCK){
                cRect.visible = false;
                collider.active = false;
                destroyed++;
                if(destroyed >= BLOCK_COUNT){
                    gamestate.hasWon = true;
                    sound.play("res/win.wav");
                }
                sound.play("res/score.wav");
            } else {
                sound.play("res/hit.wav");
            }
        }
    }

    transform.pos = glm::clamp(transform.pos, 
                        {-float(WIDTH/2)+transform.scale.x/2.0f, -float(HEIGHT)},
                        { float(WIDTH/2)-transform.scale.x/2.0f,  float(HEIGHT/2)-transform.scale.y/2.0f}
                    );
}
void scoreSystem(ECS& ecs, const Entity& entity, Component* component){
    component::Score& score = *reinterpret_cast<component::Score*>(component);
    r2d::Renderer& r2d = ecs.getResource<r2d::Renderer>("r2d");
    GameState& gamestate = ecs.getResource<GameState>("gamestate");
    if(gamestate.hasWon){
        r2d.drawSprite(score.youWonSprite);
    }
}

GLFWwindow* createWindow(int width, int height){
    shard_abort_ifnot(glfwInit());
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* win = glfwCreateWindow(
        width, height,
        "Breakout",
        nullptr, nullptr
    );
    return win;
}

void run(GLFWwindow* window, const int WIDTH, const int HEIGHT){
    r2d::Renderer r2d(window, {uint32_t(WIDTH), uint32_t(HEIGHT)}, false);
    Time time = {};
    time::updateTime(time);
    Input input(window);
    Sound sound;
    sound.setVolume(0.5f);
    randy::Random rng(size_t(time.ticks));
    GameState gamestate = {};

    const uint32_t ROW_COUNT = 3;
    const uint32_t BLOCK_COUNT = 14*ROW_COUNT;
    const float BALL_VEL = 225.0f;
    const glm::vec2 BLOCK_SCALE = {48.0f, 12.0f};
    const float BLOCK_PADDING   = 8.0f;
    const float PLAYER_PADDING  = 48.0f;

    uint32_t destroyed = 0;
    uint32_t youwinTexture = r2d.addTexture("res/youwin.png", VK_FILTER_NEAREST);

    ECS ecs;
    ComponentArray<component::Transform>  transforms(50+BLOCK_COUNT);
    ComponentArray<component::Rect>       rects(50+BLOCK_COUNT);
    ComponentArray<component::Sprite>     sprites(50);
    ComponentArray<component::Paddle>     paddles(5);
    ComponentArray<component::Controller> controllers(5);
    ComponentArray<component::Ball>       balls(5);
    ComponentArray<component::Collider>   colliders(50+BLOCK_COUNT);
    ComponentArray<component::Score>      scores(5);
    
    ecs.insertResource<r2d::Renderer>("r2d", r2d)
       .insertResource<Time>("time", time)
       .insertResource<Input>("input", input)
       .insertResource<Sound>("sound", sound)
       .insertResource<randy::Random>("rng", rng)
       .insertResource<GameState>("gamestate", gamestate)
       .insertResource<uint32_t>("youwinTexture", youwinTexture)
       .insertResource<uint32_t>("destroyed", destroyed)
       .insertConstantResource<int>("WIDTH", WIDTH)
       .insertConstantResource<int>("HEIGHT", HEIGHT)
       .insertConstantResource<float>("BALL_VEL", BALL_VEL)
       .insertConstantResource<uint32_t>("BLOCK_COUNT", BLOCK_COUNT)
       .insertComponentArray<component::Transform>("transforms", transforms)
       .insertComponentArray<component::Rect>("rects", rects)
       .insertComponentArray<component::Sprite>("sprites", sprites)
       .insertComponentArray<component::Paddle>("paddles", paddles)
       .insertComponentArray<component::Controller>("controllers", controllers)
       .insertComponentArray<component::Ball>("balls", balls)
       .insertComponentArray<component::Collider>("colliders", colliders)
       .insertComponentArray<component::Score>("scores", scores)
       .insertSystem("transformSystem", "transforms", transformSystem)
       .insertSystem("rectSystem",      "rects",      rectSystem)
       .insertSystem("spriteSystem",    "sprites",    spriteSystem)
       .insertSystem("paddleSystem",    "paddles",    paddleSystem)
       .insertSystem("ballSystem",      "balls",      ballSystem)
       .insertSystem("scoreSystem",     "scores",     scoreSystem);

    std::vector<Entity> blocks(BLOCK_COUNT);
    Entity player = ecs.addEntity();
    ecs.insertComponent<component::Transform>(
        player, "transforms", component::Transform(
            {0.0f, -float(HEIGHT/2)+(BLOCK_SCALE.y)+PLAYER_PADDING},
            0.0f,
            {BLOCK_SCALE.x*1.5f, BLOCK_SCALE.y},
            {0.0f, 0.0f}
        )).insertComponent<component::Rect>(player, "rects", component::Rect(
            {152.0f, rng.randRangef(0.0f, 255.0f), 55.0f}, 0.0f
        )).insertComponent<component::Controller>(player, "controllers", component::Controller(
            Key::A, Key::D
        )).insertComponent<component::Paddle>(player, "paddles", component::Paddle())
          .insertComponent<component::Collider>(player, "colliders", component::Collider(
              component::ColliderID::PLAYER
        )).insertComponent<component::Score>(player, "scores", component::Score());

    Entity ball   = ecs.addEntity();
    ecs.insertComponent<component::Transform>(ball, "transforms", component::Transform(
        {0.0f, 0.0f}, 0.0f, {8.0f, 8.0f}, {0.0f, -BALL_VEL}
    )).insertComponent<component::Rect>(ball, "rects", component::Rect(
        {152.0f, 55.0f, rng.randRangef(0.0f, 255.0f)}, 1.0f
    )).insertComponent<component::Ball>(ball, "balls", component::Ball());

    glm::vec2 pos = {
        -float(WIDTH/2)+(BLOCK_SCALE.x/2.0f)+BLOCK_PADDING,
        float(HEIGHT/2)-(BLOCK_SCALE.y/2.0f)-BLOCK_PADDING
    };
    for(auto& block : blocks){
        block = ecs.addEntity();
        ecs.insertComponent<component::Transform>(
            block, "transforms", component::Transform(pos, 0.0f, BLOCK_SCALE, glm::vec2{0.0f})
        ).insertComponent<component::Rect>(block, "rects", component::Rect(
            {rng.randRangef(100.0f, 255.0f), 152.0f, 104.0f},
            -1.0f
        )).insertComponent<component::Collider>(block, "colliders", component::Collider(
            component::ColliderID::BLOCK
        ));
        pos.x += BLOCK_SCALE.x+BLOCK_PADDING;
        if(pos.x > float(WIDTH/2)){
            pos.x = -float(WIDTH/2)+(BLOCK_SCALE.x/2.0f)+BLOCK_PADDING;
            pos.y -= BLOCK_SCALE.y+BLOCK_PADDING;
        }
    }

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        time::updateTime(time);
        if(r2d.startFrame({44.0f})){
            ecs.update<component::Transform>("transformSystem")
               .update<component::Paddle>   ("paddleSystem")
               .update<component::Ball>     ("ballSystem")
               .update<component::Rect>     ("rectSystem")
               .update<component::Sprite>   ("spriteSystem")
               .update<component::Score>    ("scoreSystem");
            r2d.endFrame();
        }
    }
    r2d.cleanup();
}

int main(){
    const int WIDTH=800, HEIGHT=600;
    GLFWwindow* window = createWindow(WIDTH, HEIGHT);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    run(window, WIDTH, HEIGHT);
    glfwDestroyWindow(window);
}