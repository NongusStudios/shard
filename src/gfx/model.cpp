#include <shard/gfx/model.hpp>

namespace shard{
    namespace gfx{
        Model::Model(
            Graphics& _gfx,
            const std::vector<Vertex2D>& vertices,
            const std::vector<uint32_t>& indices
        ):
            gfx{_gfx},
            vBuffer{
                gfx.createVertexBuffer(
                    vertices.size()*sizeof(gfx::Vertex2D),
                    vertices.data()
                )
            },
            iBuffer{
                gfx.createIndexBuffer(
                    indices.size()*sizeof(uint32_t),
                    indices.data()
                )
            },
            vertCount{uint32_t(vertices.size())},
            indexCount{uint32_t(indices.size())}
        {}
        Model::Model(
            Graphics& _gfx,
            const std::vector<Vertex3D>& vertices,
            const std::vector<uint32_t>& indices
        ):
            gfx{_gfx},
            vBuffer{
                gfx.createVertexBuffer(
                    vertices.size()*sizeof(gfx::Vertex3D),
                    vertices.data()
                )
            },
            iBuffer{
                gfx.createIndexBuffer(
                    indices.size()*sizeof(uint32_t),
                    indices.data()
                )
            },
            vertCount{uint32_t(vertices.size())},
            indexCount{uint32_t(indices.size())}
        {}

        void Model::bind(VkCommandBuffer cBuf){
            assert(valid());
            vBuffer.bindVertex(cBuf);
            if(iBuffer.valid())
                iBuffer.bindIndex(cBuf, VK_INDEX_TYPE_UINT32);
        }
        void Model::draw(VkCommandBuffer cBuf){
            assert(valid());
            if(iBuffer.valid()){
                vkCmdDrawIndexed(cBuf, indexCount, 1, 0, 0, 0);
                return;
            }
            vkCmdDraw(cBuf, vertCount, 1, 0, 0);
        }
    } // namespace gfx
} // namespace shard