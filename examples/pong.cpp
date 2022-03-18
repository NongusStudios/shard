#include <shard/renderer/renderer2d.hpp>
#include <shard/input/input.hpp>
#include <shard/time/time.hpp>
#include <shard/random/random.hpp>
#include <shard/sound/sound.hpp>

#define POINTS_TO_WIN 5

#define HIT_SOUND "res/hit.wav"
#define SCORE_SOUND "res/score.wav"
#define WIN_SOUND "res/win.wav"

class Pong{
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;
        static constexpr uint32_t TOP_PADDING = 48;

        Pong():
            window{createWindow(WIDTH, HEIGHT)},
            input{window},
            r2d{window, {WIDTH, HEIGHT}, 4, true}
        {
            sound.setVolume(0.5f);

            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            shard::time::updateTime(time);
            rng = shard::randy::Random(time.ticks);

            actions.player1Up    = input.addAction(shard::Key::W, 0);
            actions.player1Down  = input.addAction(shard::Key::S, 0);

            actions.player2Up    = input.addAction(shard::Key::UP,   0);
            actions.player2Down  = input.addAction(shard::Key::DOWN, 0);

            actions.reset        = input.addAction(shard::Key::R, 0);

            textures.score       = r2d.addTexture("res/scoreNumbers.png", VK_FILTER_NEAREST);
            textures.player1Wins = r2d.addTexture("res/player1Wins.png",  VK_FILTER_NEAREST);
            textures.player2Wins = r2d.addTexture("res/player2Wins.png",  VK_FILTER_NEAREST);
            textures.seperator   = r2d.addTexture("res/pongSep.png", VK_FILTER_NEAREST);
            
            renderObjects.seperator = r2d.addSprite(textures.seperator,
                {0.0f, -float(TOP_PADDING)/2.0f},
                0.0f,
                {4.0f, float(HEIGHT-TOP_PADDING)},
                {255.0f},
                0.0f
            );
            renderObjects.player1Wins = r2d.addSprite(textures.player1Wins,
                {0.0f, float(HEIGHT/2)-24.0f},
                0.0f, {128.0f, 32.0f},
                {255.0f}, 99.0f
            );
            renderObjects.player2Wins = r2d.addSprite(textures.player2Wins,
                {0.0f, float(HEIGHT/2)-24.0f},
                0.0f, {128.0f, 32.0f},
                {255.0f}, 99.0f
            );
            renderObjects.score1 = r2d.addSprite(
                textures.score,
                {-float(WIDTH/2)+22.0f, float(HEIGHT/2)-24.0f}, 0.0f,
                {32.0f, 32.0f},
                {255.0f},
                99.0f
            );
            renderObjects.score2 = r2d.addSprite(
                textures.score,
                {float(WIDTH/2)-22.0f, float(HEIGHT/2)-24.0f}, 0.0f,
                {32.0f, 32.0f},
                {255.0f},
                99.0f
            );

            renderObjects.ball = r2d.addRect(
                {0.0f, 0.0f},
                0.0f,
                {6.0f, 6.0f},
                {255.0f},
                1.0f
            );
            player1Pos = {-float(WIDTH/2)+22.0f, 0.0f};
            renderObjects.player1 = r2d.addRect(
                player1Pos,
                0.0f,
                {8.0f, 48.0f},
                {255.0f},
                0.0f
            );
            player2Pos = {float(WIDTH/2)-22.0f, 0.0f};
            renderObjects.player2 = r2d.addRect(
                player2Pos,
                0.0f,
                {8.0f, 48.0f},
                {255.0f},
                0.0f
            );
            renderObjects.decorativeBorder = r2d.addRect(
                {0.0f, float(HEIGHT/2)-TOP_PADDING+2.0f},
                0.0f,
                {float(WIDTH), 4.0f},
                {255.0f},
                99.0f
            );
        }
        ~Pong(){
            glfwDestroyWindow(window);
        }

        void processInput(){
            switch(input.actionJustPressed(actions.reset)){
                case shard::input::Result::TRUE:
                    reset();
                    break;
                default:
                    break;
            }
            switch(input.actionPressed(actions.player1Up)){
                case shard::input::Result::TRUE:
                    if(input.actionPressed(actions.player1Down) != shard::input::Result::TRUE)
                        player1Pos.y += PLAYER_VEL * time.dt;
                    break;
                default:
                    break;
            }
            switch(input.actionPressed(actions.player1Down)){
                case shard::input::Result::TRUE:
                    if(input.actionPressed(actions.player1Up) != shard::input::Result::TRUE)
                        player1Pos.y -= PLAYER_VEL * time.dt;
                    break;
                default:
                    break;
            }
            switch(input.actionPressed(actions.player2Up)){
                case shard::input::Result::TRUE:
                    if(input.actionPressed(actions.player2Down) != shard::input::Result::TRUE)
                        player2Pos.y += PLAYER_VEL * time.dt;
                    break;
                default:
                    break;
            }
            switch(input.actionPressed(actions.player2Down)){
                case shard::input::Result::TRUE:
                    if(input.actionPressed(actions.player2Up) != shard::input::Result::TRUE)
                        player2Pos.y -= PLAYER_VEL * time.dt;
                    break;
                default:
                    break;
            }
        }
        void update(){
            // Uncomment to make player2 unbeatable
            //player2Pos.y = ballPos.y;
            
            player1Pos.y
                = std::clamp(player1Pos.y, -float(HEIGHT/2)+(48.0f/2.0f), float(HEIGHT/2)-TOP_PADDING-(48.0f/2.0f));
            player2Pos.y
                = std::clamp(player2Pos.y, -float(HEIGHT/2)+(48.0f/2.0f), float(HEIGHT/2)-TOP_PADDING-(48.0f/2.0f));

            ballPos += ballVel * time.dt;
            if(ballPos.x > 0.0f && checkCollision(ballPos, {6.0f, 6.0f}, player2Pos, {4.0f, 48.0f/2.0f})){
                ballVel.x = -HIT_BALL_VEL_X;
                ballVel.y = ballVel.y >= 0.0f ? rng.randRangef(0.0f, MAX_Y_VEL) 
                                              : rng.randRangef(-MAX_Y_VEL, 0.0f);
                if(!winSoundHasPlayed)
                    sound.play(HIT_SOUND);
            } else if(ballPos.x < 0.0f && checkCollision(ballPos, {6.0f, 6.0f}, player1Pos, {4.0f, 48.0f/2.0f})){
                ballVel.x =  HIT_BALL_VEL_X;
                ballVel.y = ballVel.y >= 0.0f ? rng.randRangef(0.0f, MAX_Y_VEL)
                                              : rng.randRangef(-MAX_Y_VEL, 0.0f);
                if(!winSoundHasPlayed)
                    sound.play(HIT_SOUND);
            }

            ballPos.y = std::clamp(ballPos.y, -float(HEIGHT/2)+(6.0f/2.0f), float(HEIGHT/2)-TOP_PADDING-(6.0f/2.0f));
            if(
                ballPos.y >=  float(HEIGHT/2)-TOP_PADDING-(6.0f/2.0f) ||
                ballPos.y <= -float(HEIGHT/2)+(6.0f/2.0f)
            ){
                ballVel.y *= -1;
                if(!winSoundHasPlayed)
                    sound.play(HIT_SOUND);
            }

            if(ballPos.x > float(WIDTH/2)){
                player1Score++;
                if(player1Score < POINTS_TO_WIN){
                    ballVel = {-INIT_BALL_VEL_X, 0.0f};
                    ballPos = {0.0f, 0.0f};
                    sound.play(SCORE_SOUND);
                }
            } else if(ballPos.x < -float(WIDTH/2)){
                player2Score++;
                if(player2Score < POINTS_TO_WIN){
                    ballVel = { INIT_BALL_VEL_X, 0.0f};
                    ballPos = {0.0f, 0.0f};
                    sound.play(SCORE_SOUND);
                }
            }

            if(!winSoundHasPlayed && (player1Score >= POINTS_TO_WIN || player2Score >= POINTS_TO_WIN)){
                sound.play(WIN_SOUND);
                winSoundHasPlayed = true;
            }
        }
        void render(){
            if(r2d.startFrame({0.0f})){
                player1Score = std::clamp(player1Score, uint32_t(0), uint32_t(POINTS_TO_WIN));
                player2Score = std::clamp(player2Score, uint32_t(0), uint32_t(POINTS_TO_WIN));

                auto& score1 = r2d.getSprite(renderObjects.score1);
                score1.srcRect.position = {(16.0f+(32.0f*float(player1Score)))/(32.0f*float(POINTS_TO_WIN+1)), 0.5f};
                score1.srcRect.size = {32.0f/(32.0f*float(POINTS_TO_WIN+1)), 1.0f};

                auto& score2 = r2d.getSprite(renderObjects.score2);
                score2.srcRect.position = {(16.0f+(32.0f*float(player2Score)))/(32.0f*float(POINTS_TO_WIN+1)), 0.5f};
                score2.srcRect.size = {32.0f/(32.0f*float(POINTS_TO_WIN+1)), 1.0f};

                auto& player1 = r2d.getRect(renderObjects.player1);
                auto& player2 = r2d.getRect(renderObjects.player2);
                player1.position = player1Pos;
                player2.position = player2Pos;

                auto& ball = r2d.getRect(renderObjects.ball);
                ball.position = ballPos;
                
                r2d.drawRect(  renderObjects.ball   )
                   .drawRect(  renderObjects.player1)
                   .drawRect(  renderObjects.player2)
                   .drawRect(  renderObjects.decorativeBorder)
                   .drawSprite(renderObjects.seperator)
                   .drawSprite(renderObjects.score1 )
                   .drawSprite(renderObjects.score2 );
                if(player1Score == uint32_t(POINTS_TO_WIN)){
                    r2d.drawSprite(renderObjects.player1Wins);
                } else if(player2Score == uint32_t(POINTS_TO_WIN)){
                    r2d.drawSprite(renderObjects.player2Wins);
                }
                r2d.endFrame();
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
            r2d.cleanup();
        }

        bool checkCollision(glm::vec2 pos1, glm::vec2 size1, glm::vec2 pos2, glm::vec2 size2){
            return  pos1.x  < pos2.x  + size2.x &&
                    pos1.x  + size1.x > pos2.x  &&
                    pos1.y  < pos2.y  + size2.y &&
                    size2.y + pos1.y  > pos2.y;
        }

        void reset(){
            ballPos = {0.0f, 0.0f};
            ballVel = {-INIT_BALL_VEL_X, 0.0f};
            player1Score = 0;
            player2Score = 0;
            winSoundHasPlayed = false;
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
        shard::Sound sound;
        shard::Time time;
        shard::randy::Random rng;
        
        struct{
            uint32_t player1Up;
            uint32_t player2Up;
            uint32_t player1Down;
            uint32_t player2Down;
            uint32_t reset;
        } actions;

        struct{
            uint32_t ball;
            uint32_t player1;
            uint32_t player2;
            uint32_t score1;
            uint32_t score2;
            uint32_t player1Wins;
            uint32_t player2Wins;
            uint32_t decorativeBorder;
            uint32_t seperator;
        } renderObjects;
        struct{
            uint32_t score;
            uint32_t player1Wins;
            uint32_t player2Wins;
            uint32_t seperator;
        } textures;
        bool winSoundHasPlayed = false;

        const float PLAYER_VEL = 200.0f;
        uint32_t player1Score = 0;
        glm::vec2 player1Pos = {0.0f, 0.0f};
        uint32_t player2Score = 0;
        glm::vec2 player2Pos = {0.0f, 0.0f};

        const float INIT_BALL_VEL_X = 150.0f;
        const float HIT_BALL_VEL_X = 500.0f;
        const float MAX_Y_VEL = 200.0f;
        glm::vec2 ballPos = { 0.0f, 0.0f };
        glm::vec2 ballVel = {-INIT_BALL_VEL_X, 0.0f};
};

int main(){
    Pong pong;
    pong.run();
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