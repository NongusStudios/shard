#include <shard/renderer/renderer2d.hpp>

namespace shard{
    struct PushConstantData{
        glm::mat4 model;
        glm::mat4 view;  
    };

    namespace r2d{
        const gfx::Vertex2D Rect::VERTICES[4] = {
            //            pos            
            gfx::Vertex2D({-1.0f, -1.0f}, {0.0f, 0.0f}, {1.0f}),
            gfx::Vertex2D({ 1.0f,  1.0f}, {1.0f, 1.0f}, {1.0f}),
            gfx::Vertex2D({-1.0f,  1.0f}, {0.0f, 1.0f}, {1.0f}),
            gfx::Vertex2D({ 1.0f, -1.0f}, {1.0f, 0.0f}, {1.0f})
        };
        const uint32_t      Rect::INDICES[6] = {
            0, 1, 2,
            0, 3, 1
        };
        const gfx::Vertex2D Circle::VERTICES[4] = {
            //            pos            
            gfx::Vertex2D({-1.0f, -1.0f}, {0.0f, 0.0f}, {1.0f}),
            gfx::Vertex2D({ 1.0f,  1.0f}, {1.0f, 1.0f}, {1.0f}),
            gfx::Vertex2D({-1.0f,  1.0f}, {0.0f, 1.0f}, {1.0f}),
            gfx::Vertex2D({ 1.0f, -1.0f}, {1.0f, 0.0f}, {1.0f})
        };
        const uint32_t      Circle::INDICES[6] = {
            0, 1, 2,
            0, 3, 1
        };
    } // namespace r2d
} // namespace shard