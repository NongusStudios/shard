#pragma once

#include <map>
#include <vector>
#include <string>
#include <cstdint>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../def.hpp"

namespace shard {
    enum class Key{
        ESCAPE = GLFW_KEY_ESCAPE,
        F1 = GLFW_KEY_F1,
        F2 = GLFW_KEY_F2, 
        F3 = GLFW_KEY_F3, 
        F4 = GLFW_KEY_F4, 
        F5 = GLFW_KEY_F5,
        F6 = GLFW_KEY_F6, 
        F7 = GLFW_KEY_F7, 
        F8 = GLFW_KEY_F8, 
        F9 = GLFW_KEY_F9, 
        F10 = GLFW_KEY_F10, 
        F11 = GLFW_KEY_F11, 
        F12 = GLFW_KEY_F12, 
        F13 = GLFW_KEY_F13, 
        F14 = GLFW_KEY_F14, 
        F15 = GLFW_KEY_F15, 
        F16 = GLFW_KEY_F16,
        F17 = GLFW_KEY_F17,
        F18 = GLFW_KEY_F18,
        F19 = GLFW_KEY_F19,
        F20 = GLFW_KEY_F20,
        F21 = GLFW_KEY_F21,
        F22 = GLFW_KEY_F22,
        F23 = GLFW_KEY_F23,
        F24 = GLFW_KEY_F24,
        F25 = GLFW_KEY_F25,        
        PRINT_SCREEN = GLFW_KEY_PRINT_SCREEN,
        SCROLL_LOCK = GLFW_KEY_SCROLL_LOCK,
        PAUSE_BREAK = GLFW_KEY_PAUSE,
        GRAVE_ACCENT = GLFW_KEY_GRAVE_ACCENT,
        N1 = GLFW_KEY_1,
        N2 = GLFW_KEY_2,
        N3 = GLFW_KEY_3,
        N4 = GLFW_KEY_4,
        N5 = GLFW_KEY_5,
        N6 = GLFW_KEY_6,
        N7 = GLFW_KEY_7,
        N8 = GLFW_KEY_8,
        N9 = GLFW_KEY_9,
        N0 = GLFW_KEY_0,
        MINUS = GLFW_KEY_MINUS,
        EQUAL = GLFW_KEY_EQUAL,
        BACKSPACE = GLFW_KEY_BACKSPACE,
        INS = GLFW_KEY_INSERT,
        HOME = GLFW_KEY_HOME,
        PGUP = GLFW_KEY_PAGE_UP,
        NUMLOCK = GLFW_KEY_NUM_LOCK,
        NP_DIVIDE = GLFW_KEY_KP_DIVIDE,
        NP_MULTIPLY = GLFW_KEY_KP_MULTIPLY,
        NP_SUBTRACT = GLFW_KEY_KP_SUBTRACT,
        TAB = GLFW_KEY_TAB,
        Q = GLFW_KEY_Q,
        W = GLFW_KEY_W,
        E = GLFW_KEY_E,
        R = GLFW_KEY_R,
        T = GLFW_KEY_T,
        Y = GLFW_KEY_Y,
        U = GLFW_KEY_U,
        I = GLFW_KEY_I,
        O = GLFW_KEY_O,
        P = GLFW_KEY_P,
        SQR_BRACKET_LEFT = GLFW_KEY_LEFT_BRACKET,
        SQR_BRACKET_RIGHT = GLFW_KEY_RIGHT_BRACKET,
        BACKSLASH = GLFW_KEY_BACKSLASH,
        DEL = GLFW_KEY_DELETE,
        END = GLFW_KEY_END,
        PGDN = GLFW_KEY_PAGE_DOWN,
        NP_7 = GLFW_KEY_KP_7,
        NP_8 = GLFW_KEY_KP_8,
        NP_9 = GLFW_KEY_KP_9,
        NP_ADD = GLFW_KEY_KP_ADD,
        CAPSLOCK = GLFW_KEY_CAPS_LOCK,
        A = GLFW_KEY_A,
        S = GLFW_KEY_S,
        D = GLFW_KEY_D,
        F = GLFW_KEY_F,
        G = GLFW_KEY_G,
        H = GLFW_KEY_H,
        J = GLFW_KEY_J,
        K = GLFW_KEY_K,
        L = GLFW_KEY_L,
        SEMICOLON = GLFW_KEY_SEMICOLON,
        APOSTROPHE = GLFW_KEY_APOSTROPHE,
        ENTER = GLFW_KEY_ENTER,
        NP_4 = GLFW_KEY_KP_4,
        NP_5 = GLFW_KEY_KP_5,
        NP_6 = GLFW_KEY_KP_6,
        LSHIFT = GLFW_KEY_LEFT_SHIFT,
        Z = GLFW_KEY_Z,
        X = GLFW_KEY_X,
        C = GLFW_KEY_C,
        V = GLFW_KEY_V,
        B = GLFW_KEY_B,
        N = GLFW_KEY_N,
        M = GLFW_KEY_M,
        COMMA = GLFW_KEY_COMMA,
        PERIOD = GLFW_KEY_PERIOD,
        SLASH = GLFW_KEY_SLASH,
        RSHIFT = GLFW_KEY_RIGHT_SHIFT,
        UP = GLFW_KEY_UP,
        NP_1 = GLFW_KEY_KP_1,
        NP_2 = GLFW_KEY_KP_2,
        NP_3 = GLFW_KEY_KP_3,
        NP_ENTER = GLFW_KEY_KP_ENTER,
        LCTRL = GLFW_KEY_LEFT_CONTROL,
        LSUPER = GLFW_KEY_LEFT_SUPER,
        LALT = GLFW_KEY_LEFT_ALT,
        SPACE = GLFW_KEY_SPACE,
        RALT = GLFW_KEY_RIGHT_ALT,
        RCTRL = GLFW_KEY_RIGHT_CONTROL,
        LEFT = GLFW_KEY_LEFT,
        DOWN = GLFW_KEY_DOWN,
        RIGHT = GLFW_KEY_RIGHT,
        NP_0 = GLFW_KEY_KP_0
    };
    enum class MouseButton{
        B1  = GLFW_MOUSE_BUTTON_1,
        B2  = GLFW_MOUSE_BUTTON_2,
        B3  = GLFW_MOUSE_BUTTON_3,
        B4  = GLFW_MOUSE_BUTTON_4,
        B5  = GLFW_MOUSE_BUTTON_5,
        B6  = GLFW_MOUSE_BUTTON_6,
        B7  = GLFW_MOUSE_BUTTON_7,
        B8  = GLFW_MOUSE_BUTTON_8,
    };

    namespace input {
        enum class ActionType{
            KEY,
            BUTTON
        };
        enum class Result{
            FALSE,
            TRUE,
            DISABLED
        };
        struct Action{
            Action():
                action{0},
                type{ActionType::KEY},
                layers{std::initializer_list<uint32_t>{0}},
                pressed{false},
                released{true}
            {}
            Action(Key key, const std::vector<uint32_t>& _layers):
                action{static_cast<int>(key)},
                type{ActionType::KEY},
                layers{_layers},
                pressed{false},
                released{true}
            {}
            Action(MouseButton button, const std::vector<uint32_t>& _layers):
                action{static_cast<int>(button)},
                type{ActionType::BUTTON},
                layers{_layers},
                pressed{false},
                released{true}
            {}

            int action;
            ActionType type;
            std::vector<uint32_t> layers;

            bool pressed;
            bool released;
        };
        static const MouseButton MB_LEFT   = MouseButton::B1;
        static const MouseButton MB_RIGHT  = MouseButton::B2;
        static const MouseButton MB_MIDDLE = MouseButton::B3;
    } // namespace input

    class Input{
        private:
            GLFWwindow* window;
            std::map<uint32_t, uint8_t> activeLayers;
            std::map<uint32_t, input::Action> actions;
            uint32_t currentActionID = 0;

            int getAction(const input::Action& action);
            bool actionIsActive(const input::Action& action);
        public:
            Input(GLFWwindow* _window);

            Input(const Input&) = delete;
            Input& operator = (const Input&) = delete;

            uint32_t addAction(Key key, uint32_t layer);
            uint32_t addAction(Key key, const std::vector<uint32_t>& layers);
            uint32_t addAction(MouseButton button, uint32_t layer);
            uint32_t addAction(MouseButton button, const std::vector<uint32_t>& layers);

            void setAction(const uint32_t action, Key key, uint32_t layer);
            void setAction(const uint32_t action, Key key, const std::vector<uint32_t>& layers);
            void setAction(const uint32_t action, MouseButton button, uint32_t layer);
            void setAction(const uint32_t action, MouseButton button, const std::vector<uint32_t>& layers);

            // Returns GLFW_PRESS / GLFW_RELEASE
            int getAction(const uint32_t action);
            bool actionIsActive(const uint32_t action);
            input::Result actionPressed(const uint32_t action);
            input::Result actionJustPressed(const uint32_t action);
            input::Result actionReleased(const uint32_t action);
            input::Result actionJustReleased(const uint32_t action);

            void activate(  uint32_t layer);
            void deactivate(uint32_t layer);
            void activate(  const std::vector<uint32_t>& layers);
            void deactivate(const std::vector<uint32_t>& layers);
    };
} // namespace shard

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