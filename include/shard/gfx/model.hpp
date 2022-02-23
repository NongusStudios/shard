#pragma once

#include "gfx.hpp"

namespace shard{
    namespace gfx{
        class Model{
            public:
                Model(
                    Graphics& _gfx,
                    const std::vector<Vertex2D>& vertices,
                    const std::vector<uint32_t>& indices
                );
                Model(
                    Graphics& _gfx,
                    const std::vector<Vertex3D>& vertices,
                    const std::vector<uint32_t>& indices
                );

                void bind(VkCommandBuffer cBuf);
                void draw(VkCommandBuffer cBuf);
                
                bool valid(){ return vBuffer.valid(); }
            private:
                Graphics& gfx;
                Buffer    vBuffer;
                Buffer    iBuffer;
                uint32_t  vertCount;
                uint32_t  indexCount;
        };
    }
}