#include <jate/components/render_units/rect2d_render_unit.h>

#include <jate/models/entity.h>

namespace jate::components
{
    void Rect2DRenderUnit::setRect(float centerX, float centerY, float width, float height)
    {
        m_centerX = centerX;
        m_centerY = centerY;
        m_width = width;
        m_height = height;
    }

    ARenderUnit::AllocatedRenderingData Rect2DRenderUnit::allocateRenderingData(rendering::ARenderer* renderer) const
    {
        glm::vec3 color = {1.f, 1.f, 1.f};
        float posZ = m_entity->getTransform().position.z;
        float extentX = m_width / 2.f;
        float extentY = m_height / 2.f;

        std::vector<rendering::VertexData> vertices
        {
            {{m_centerX - extentX,  m_centerY + extentY, posZ}, color},
            {{m_centerX - extentX,  m_centerY - extentY, posZ}, color},
            {{m_centerX + extentX,  m_centerY - extentY, posZ}, color},
            {{m_centerX + extentX,  m_centerY + extentY, posZ}, color}
        };

        std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

        return {
            .verticesSlot = renderer->allocateVertexData(vertices),
            .indicesSlot = renderer->allocateIndexData(indices)
        };
    }
}