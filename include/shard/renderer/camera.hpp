#pragma once

#include "../gfx/gfx.hpp"

namespace shard{
    namespace r2d{
        class Camera{
            public:
                Camera():
                    position{},
                    zoom{}
                {}
                Camera(const glm::vec2& pos, const glm::vec2& _zoom):
                    position{pos},
                    zoom{_zoom}
                {}

                glm::mat4 viewMatrix(){
                    glm::mat4 view = glm::mat4(1.0f);
                    view = glm::translate(view,
                        {
                            -position.x*zoom.x,    // x
                            -position.y*zoom.y,    // y
                            0.0f // z
                        }
                    );
                    return view;
                }
                glm::vec2 position;
                glm::vec2 zoom;
        };
    } // namespace r2d
} // namespace shard