#pragma once

#include "gfx.hpp"

namespace shard{
    namespace gfx{
        class Model{
            public:
                Model(Graphics& _gfx);
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
                Model(
                    Graphics& _gfx,
                    const Vertex2D* vertices, size_t vcount,
                    const uint32_t* indices,  size_t icount
                );
                Model(
                    Graphics& _gfx,
                    const Vertex3D* vertices, size_t vcount,
                    const uint32_t* indices,  size_t icount
                );
                Model(Model&  m);
                Model(Model&& m);
                
                shard_delete_copy_constructors(Model);
                
                Model& operator = (Model&  m);
                Model& operator = (Model&& m);

                void bind(VkCommandBuffer commandBuffer);
                void draw(VkCommandBuffer commandBuffer);
                
                Buffer& vertexBuffer(){ return vBuffer; }
                Buffer& indexBuffer() { return iBuffer; }
                const Buffer& vertexBuffer() const { return vBuffer; }
                const Buffer& indexBuffer()  const { return iBuffer; }
                uint32_t vertexCount() const { return vertCount; }
                uint32_t indexCount()  const { return _indexCount; }

                bool valid(){ return vBuffer.valid(); }
            private:
                Graphics& gfx;
                Buffer    vBuffer;
                Buffer    iBuffer;
                uint32_t  vertCount;
                uint32_t  _indexCount;
        };
    }
}