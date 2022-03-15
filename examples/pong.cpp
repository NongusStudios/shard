#include <shard/renderer/renderer2d.hpp>
#include <shard/input/input.hpp>
#include <shard/time/time.hpp>
#include <shard/random/random.hpp>

class Pong{
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        Pong():
            window{createWindow(WIDTH, HEIGHT)},
            input{window},
            r2d{window, {800, 600}, 1, true}
        {
            shard::time::updateTime(time);
            rng = shard::randy::Random(time.ticks);

            actions.player1Up   = input.addAction(shard::Key::W, 0);
            actions.player1Down = input.addAction(shard::Key::S, 0);

            actions.player2Up   = input.addAction(shard::Key::UP,   0);
            actions.player2Down = input.addAction(shard::Key::DOWN, 0);

            renderObjects.ball = r2d.addRect(
                {0.0f, 0.0f},
                0.0f,
                {6.0f, 6.0f},
                {255.0f},
                0.0f
            );
            renderObjects.player1 = r2d.addRect(
                {-float(WIDTH/2)+22.0f, 0.0f},
                0.0f,
                {8.0f, 48.0f},
                {255.0f},
                0.0f
            );
            renderObjects.player2 = r2d.addRect(
                {float(WIDTH/2)-22.0f, 0.0f},
                0.0f,
                {8.0f, 48.0f},
                {255.0f},
                0.0f
            );
        }
        ~Pong(){
            glfwDestroyWindow(window);
        }

        void update(){
            shard::r2d::Rect& player1 = r2d.getRect(renderObjects.player1);
            shard::r2d::Rect& player2 = r2d.getRect(renderObjects.player2);
            switch(input.actionPressed(actions.player1Up)){
                case shard::input::Result::TRUE:
                    player1.position.y += PLAYER_SPEED * time.dt;
                    break;
                default:
                    break;
            }
            switch(input.actionPressed(actions.player1Down)){
                case shard::input::Result::TRUE:
                    player1.position.y -= PLAYER_SPEED * time.dt;
                    break;
                default:
                    break;
            }
            switch(input.actionPressed(actions.player2Up)){
                case shard::input::Result::TRUE:
                    player2.position.y += PLAYER_SPEED * time.dt;
                    break;
                default:
                    break;
            }
            switch(input.actionPressed(actions.player2Down)){
                case shard::input::Result::TRUE:
                    player2.position.y -= PLAYER_SPEED * time.dt;
                    break;
                default:
                    break;
            }

            player1.position.y = glm::clamp(
                player1.position.y, -float(HEIGHT/2)+player1.scale.y, float(HEIGHT/2)-player1.scale.y
            );
            player2.position.y = glm::clamp(
                player2.position.y, -float(HEIGHT/2)+player2.scale.y, float(HEIGHT/2)-player2.scale.y
            );

            shard::r2d::Rect& ball = r2d.getRect(renderObjects.ball);
            ball.position += ballVel * time.dt;

            if(ball.position.x > 0){
                if(checkCollision(ball.position, ball.scale, player2.position, player2.scale)){
                    ballVel = -ballVel * VEL_MUL;
                    ballVel.y = rng.randRangef(-MAX_Y_VEL, MAX_Y_VEL);
                }
            } else{
                if(checkCollision(ball.position, ball.scale, player1.position, player1.scale)){
                    ballVel = -ballVel * VEL_MUL;
                    ballVel.y = rng.randRangef(-MAX_Y_VEL, MAX_Y_VEL);
                }
            }

            if(ball.position.y > float(HEIGHT/2) || ball.position.y < -float(HEIGHT/2)){
                ballVel.y = -ballVel.y;
            }

            if(ball.position.x > float(WIDTH/2)){
                player1Score++;
                ball.position = {0.0f, 0.0f};
                ballVel = BASE_BALL_VEL;
                printScore();
            } else if(ball.position.x < -float(WIDTH/2)){
                player2Score++;
                ball.position = {0.0f, 0.0f};
                ballVel = BASE_BALL_VEL;
                printScore();
            }
        }
        void render(){
            if(r2d.startFrame({0.0f})){
                r2d.drawRect(renderObjects.ball)
                   .drawRect(renderObjects.player1)
                   .drawRect(renderObjects.player2);
                r2d.endFrame();
            }
        }

        void run(){
            while(!glfwWindowShouldClose(window)){
                glfwPollEvents();
                shard::time::updateTime(time);
                update();
                render();
            }
            r2d.cleanup();
        }

        void printScore(){
            std::cout << "Player1: " << player1Score << ", Player2: " << player2Score << "\n";
        }

        bool checkCollision(glm::vec2 pos1, glm::vec2 size1, glm::vec2 pos2, glm::vec2 size2){
            return  pos1.x  < pos2.x  + size2.x &&
                    pos1.x  + size1.x > pos2.x  &&
                    pos1.y  < pos2.y  + size2.y &&
                    size2.y + pos1.y  > pos2.y;
        }
    private:
        GLFWwindow* createWindow(int w, int h){
            shard_abort_ifnot(glfwInit());
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            GLFWwindow* win = glfwCreateWindow(
                w, h,
                "Pong",
                nullptr, nullptr
            );
            return win;
        }
        GLFWwindow* window;
        shard::Input input;
        shard::r2d::Renderer r2d;
        shard::Time time;
        shard::randy::Random rng;
        
        struct{
            uint32_t player1Up;
            uint32_t player2Up;
            uint32_t player1Down;
            uint32_t player2Down;
        } actions;

        struct{
            uint32_t ball;
            uint32_t player1;
            uint32_t player2;
        } renderObjects;

        const float PLAYER_SPEED = 300.0f;
        const glm::vec2 BASE_BALL_VEL = glm::vec2{-150.0f, 0.0f};
        const float VEL_MUL = 1.1f;
        const float MAX_Y_VEL = 150.0f;
        glm::vec2 ballVel = BASE_BALL_VEL;

        uint32_t player1Score = 0;
        uint32_t player2Score = 0;
};

int main(){
    Pong pong;
    pong.run();
    return 0;
}