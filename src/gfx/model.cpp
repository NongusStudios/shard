#include <shard/gfx/model.hpp>

namespace shard{
    namespace gfx{
        Model::Model(Graphics& _gfx):
            gfx{_gfx},
            vBuffer{gfx.device()},
            iBuffer{gfx.device()},
            vertCount{0},
            _indexCount{0}
        {}
        Model::Model(
            Graphics& _gfx,
            const std::vector<Vertex2D>& vertices,
            const std::vector<uint32_t>& indices
        ):
            gfx{_gfx},
            vBuffer{
                gfx.createVertexBuffer(
                    vertices.size()*sizeof(Vertex2D), VK_SHARING_MODE_EXCLUSIVE,
                    vertices.data()
                )
            },
            iBuffer{
                gfx.createIndexBuffer(
                    indices.size()*sizeof(uint32_t), VK_SHARING_MODE_EXCLUSIVE,
                    indices.data()
                )
            },
            vertCount{uint32_t(vertices.size())},
            _indexCount{uint32_t(indices.size())}
        {}
        Model::Model(
            Graphics& _gfx,
            const std::vector<Vertex3D>& vertices,
            const std::vector<uint32_t>& indices
        ):
            gfx{_gfx},
            vBuffer{
                gfx.createVertexBuffer(
                    vertices.size()*sizeof(Vertex3D), VK_SHARING_MODE_EXCLUSIVE,
                    vertices.data()
                )
            },
            iBuffer{
                gfx.createIndexBuffer(
                    indices.size()*sizeof(uint32_t), VK_SHARING_MODE_EXCLUSIVE,
                    indices.data()
                )
            },
            vertCount{uint32_t(vertices.size())},
            _indexCount{uint32_t(indices.size())}
        {}
        Model::Model(
            Graphics& _gfx,
            const Vertex2D* vertices, size_t vcount,
            const uint32_t* indices,  size_t icount
        ):
            gfx{_gfx},
            vBuffer{
                gfx.createVertexBuffer(
                    vcount*sizeof(Vertex2D), VK_SHARING_MODE_EXCLUSIVE,
                    vertices
                )
            },
            iBuffer{
                gfx.createIndexBuffer(
                    icount*sizeof(uint32_t), VK_SHARING_MODE_EXCLUSIVE,
                    indices
                )
            },
            vertCount{uint32_t(vcount)},
            _indexCount{uint32_t(icount)}
        {}
        Model::Model(
            Graphics& _gfx,
            const Vertex3D* vertices, size_t vcount,
            const uint32_t* indices,  size_t icount
        ):
            gfx{_gfx},
            vBuffer{
                gfx.createVertexBuffer(
                    vcount*sizeof(Vertex3D), VK_SHARING_MODE_EXCLUSIVE,
                    vertices
                )
            },
            iBuffer{
                gfx.createIndexBuffer(
                    icount*sizeof(uint32_t), VK_SHARING_MODE_EXCLUSIVE,
                    indices
                )
            },
            vertCount{uint32_t(vcount)},
            _indexCount{uint32_t(icount)}
        {}
        Model::Model(Model&  m):
            gfx{m.gfx},
            vBuffer{m.vBuffer},
            iBuffer{m.iBuffer},
            vertCount{m.vertCount},
            _indexCount{m._indexCount}
        {}
        Model::Model(Model&& m):
            gfx{m.gfx},
            vBuffer{m.vBuffer},
            iBuffer{m.iBuffer},
            vertCount{m.vertCount},
            _indexCount{m._indexCount}
        {}
        Model& Model::operator = (Model&  m){
            assert(&gfx == &m.gfx);
            vBuffer = m.vBuffer;
            iBuffer = m.iBuffer;
            vertCount = m.vertCount;
            _indexCount = m._indexCount;
            return *this;
        }
        Model& Model::operator = (Model&& m){
            assert(&gfx == &m.gfx);
            vBuffer = m.vBuffer;
            iBuffer = m.iBuffer;
            vertCount = m.vertCount;
            _indexCount = m._indexCount;
            return *this;
        }

        void Model::bind(VkCommandBuffer cBuf){
            assert(valid());
            vBuffer.bindVertex(cBuf);
            if(iBuffer.valid())
                iBuffer.bindIndex(cBuf, VK_INDEX_TYPE_UINT32);
        }
        void Model::draw(VkCommandBuffer cBuf){
            assert(valid());
            if(iBuffer.valid()){
                vkCmdDrawIndexed(cBuf, _indexCount, 1, 0, 0, 0);
                return;
            }
            vkCmdDraw(cBuf, vertCount, 1, 0, 0);
        }
    } // namespace gfx
} // namespace shard