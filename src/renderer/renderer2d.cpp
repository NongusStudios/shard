#include <shard/renderer/renderer2d.hpp>

namespace shard{
    struct PushConstantData{
        
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
    } // namespace r2d
    

    Renderer2D::Renderer2D(GLFWwindow* _window, VkExtent2D _renderRes):
        window{_window},
        renderRes{_renderRes},
        gfx{window},
        rectVertexBuffer{
            gfx.createVertexBuffer(
                sizeof(r2d::Rect::VERTICES),
                r2d::Rect::VERTICES
            )
        },
        rectIndexBuffer{
            gfx.createIndexBuffer(
                sizeof(r2d::Rect::INDICES),
                r2d::Rect::INDICES
            )
        }
    {}
    Renderer2D::~Renderer2D(){}

    bool Renderer2D::begin(const gfx::Color& color){
        if(currentCommandBuffer = gfx.beginRenderPass(color)){
            return true;
        }
        return false;
    }
    void Renderer2D::present(){
        assert(currentCommandBuffer != VK_NULL_HANDLE);
        gfx.endRenderPass();
        currentCommandBuffer = VK_NULL_HANDLE;
    }
} // namespace shard