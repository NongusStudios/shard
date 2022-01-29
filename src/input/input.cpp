#include <shard/input/input.hpp>

#include <cassert>
#include <iostream>

namespace shard {
    Input::Input(GLFWwindow* _window):
        window{_window}
    { assert(window); }

    uint32_t Input::addAction(Key key, uint32_t layer){
        return addAction(key, std::initializer_list<uint32_t>{layer});
    }
    uint32_t Input::addAction(Key key, const std::vector<uint32_t>& layers){
        assert(layers.size()>0);
        actions[currentActionID] = {
            key,
            layers
        };
        return currentActionID++;
    }
    uint32_t Input::addAction(MouseButton button, uint32_t layer){
        return addAction(button, std::initializer_list<uint32_t>{layer});
    }
    uint32_t Input::addAction(MouseButton button, const std::vector<uint32_t>& layers){
        assert(layers.size()>0);
        actions[currentActionID] = {
            button,
            layers
        };
        return currentActionID++;
    }

    void Input::setAction(uint32_t action, Key key, uint32_t layer){
        setAction(action, key, std::initializer_list<uint32_t>{layer});
    }
    void Input::setAction(uint32_t action, Key key, const std::vector<uint32_t>& layers){
        assert(actions.contains(action));
        actions[action] = {
            key,
            layers
        };
    }
    void Input::setAction(uint32_t action, MouseButton button, uint32_t layer){
        setAction(action, button, std::initializer_list<uint32_t>{layer});
    }
    void Input::setAction(uint32_t action, MouseButton button, const std::vector<uint32_t>& layers){
         assert(actions.contains(action));
         actions[action] = {
            button,
            layers
        };
    }

    int Input::getAction(const input::Action& action){
        switch(action.type){
            case input::ActionType::KEY:
                return glfwGetKey(window, action.action);
            case input::ActionType::BUTTON:
                return glfwGetMouseButton(window, action.action);
            default:
                return GLFW_RELEASE;
        };
    }
    int Input::getAction(const uint32_t action){
        assert(actions.contains(action));
        const input::Action& realAction = actions[action];
        return getAction(realAction);
    }
    bool Input::actionIsActive(const input::Action& action){
        for(const auto& layer : action.layers){
            if(layer == 0) continue;
            if(!activeLayers.contains(layer)) return false;
        }
        return true;
    }
    bool Input::actionIsActive(const uint32_t action){
        assert(actions.contains(action));
        const input::Action& realAction = actions[action];
        return actionIsActive(realAction);
    }
    input::Result Input::actionPressed(uint32_t action){
        assert(actions.contains(action));
        const input::Action& realAction = actions[action];
        if(!actionIsActive(realAction)) return input::Result::DISABLED;
        switch(getAction(realAction)){
            case GLFW_PRESS:
                return input::Result::TRUE;
            case GLFW_RELEASE:
                return input::Result::FALSE;
            default:
                return input::Result::DISABLED;
        }
    }
    input::Result Input::actionJustPressed(uint32_t action){
        assert(actions.contains(action));
        input::Action& realAction = actions[action];
        switch(actionPressed(action)){
            case input::Result::TRUE:
                if(!realAction.pressed){
                    realAction.pressed = true;
                    return input::Result::TRUE;
                }
                return input::Result::FALSE;
            case input::Result::FALSE:
                realAction.pressed = false;
                return input::Result::FALSE;
            case input::Result::DISABLED:
                return input::Result::DISABLED;
        }
        return input::Result::DISABLED;
    }
    input::Result Input::actionReleased(uint32_t action){
        assert(actions.contains(action));
        switch(actionPressed(action)){
            case input::Result::TRUE:
                return input::Result::FALSE;
            case input::Result::FALSE:
                return input::Result::TRUE;
            case input::Result::DISABLED:
                return input::Result::DISABLED;
        }
        return input::Result::DISABLED;
    }
    input::Result Input::actionJustReleased(uint32_t action){
        assert(actions.contains(action));
        input::Action& realAction = actions[action];
        switch(actionPressed(action)){
            case input::Result::TRUE:
                realAction.released = false;
                return input::Result::FALSE;
            case input::Result::FALSE:
                if(!realAction.released){
                    realAction.released = true;
                    return input::Result::TRUE;
                }
                return input::Result::FALSE;
            case input::Result::DISABLED:
                return input::Result::DISABLED;
        }
        return input::Result::DISABLED;
    }

    void Input::activate(  uint32_t layer){
        assert(layer != 0 && "Cannot change base layer state!");
        activeLayers[layer] = 0;
    }
    void Input::deactivate(uint32_t layer){
        assert(layer != 0 && "Cannot change base layer state!");
        activeLayers.erase(layer);
    }
    void Input::activate(  const std::vector<uint32_t>& layers){
        for(const auto& layer : layers){
            activate(layer);
        }
    }
    void Input::deactivate(const std::vector<uint32_t>& layers){
        for(const auto& layer : layers){
            deactivate(layer);
        }
    }
} // namespace shard